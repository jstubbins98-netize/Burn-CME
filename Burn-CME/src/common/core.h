#ifndef BURN_CME_CORE_H
#define BURN_CME_CORE_H

#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <set>
#include <memory>
#include <array>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <regex>
#include <fstream>

namespace fs = std::filesystem;

enum class Platform {
    Linux,
    macOS,
    Unknown
};

struct DriveInfo {
    std::string path;
    std::string name;
    std::string capabilities;
    bool is_apple_superdrive = false;
    std::string usb_path;
};

struct FileEntry {
    std::string path;
    std::string display_name;
    size_t size;
    bool is_directory;
};

class BurnCMECore {
public:
    using LogCallback = std::function<void(const std::string&)>;
    using ProgressCallback = std::function<void(int)>;

private:
    std::vector<FileEntry> file_queue;
    std::vector<std::string> paths_added;
    std::string current_drive;
    std::string output_directory;
    LogCallback log_callback;
    ProgressCallback progress_callback;
    Platform platform;
    bool current_drive_is_superdrive = false;

    std::string shell_escape(const std::string& input) {
        std::string result = "'";
        for (char c : input) {
            if (c == '\'') {
                result += "'\"'\"'";
            } else {
                result += c;
            }
        }
        result += "'";
        return result;
    }

    std::string execute_command(const std::string& cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
            return "Error: Failed to execute command";
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    int execute_command_status(const std::string& cmd) {
        return system(cmd.c_str());
    }

    void log(const std::string& msg) {
        if (log_callback) {
            log_callback(msg);
        }
    }

    std::string generate_unique_name(const std::string& base_name, 
                                      const std::set<std::string>& existing_names) {
        if (existing_names.find(base_name) == existing_names.end()) {
            return base_name;
        }
        int counter = 1;
        std::string new_name;
        do {
            new_name = base_name + "_" + std::to_string(counter++);
        } while (existing_names.find(new_name) != existing_names.end());
        return new_name;
    }

    Platform detectPlatform() {
        #ifdef __APPLE__
            return Platform::macOS;
        #elif __linux__
            return Platform::Linux;
        #else
            return Platform::Unknown;
        #endif
    }

    std::string getSuperDriveForDevice(const std::string& device) {
        if (platform != Platform::Linux) return "";
        
        std::string sr_name = fs::path(device).filename().string();
        
        std::string sys_path = "/sys/class/block/" + sr_name + "/device";
        if (!fs::exists(sys_path)) return "";
        
        try {
            fs::path device_path = fs::read_symlink(sys_path);
            std::string full_path = device_path.string();
            
            std::string usb_path = full_path;
            size_t usb_pos = usb_path.find("/usb");
            if (usb_pos != std::string::npos) {
                size_t end_pos = usb_path.find("/", usb_pos + 5);
                if (end_pos != std::string::npos) {
                    usb_path = usb_path.substr(0, end_pos);
                }
            }
            
            std::string vendor_file = usb_path + "/idVendor";
            std::string product_file = usb_path + "/idProduct";
            
            if (fs::exists(vendor_file) && fs::exists(product_file)) {
                std::ifstream vf(vendor_file);
                std::ifstream pf(product_file);
                std::string vendor, product;
                vf >> vendor;
                pf >> product;
                
                if (vendor == "05ac" && product == "1500") {
                    return device;
                }
            }
        } catch (...) {
        }
        
        return "";
    }

    bool isAppleSuperDriveDevice(const std::string& device) {
        return !getSuperDriveForDevice(device).empty();
    }

    bool wakeAppleSuperDrive(const std::string& device) {
        if (platform != Platform::Linux) return true;
        if (!current_drive_is_superdrive) return true;
        
        log("Apple SuperDrive detected - sending wake-up command...");
        
        std::string sg_raw_check = execute_command("which sg_raw 2>/dev/null");
        if (sg_raw_check.empty() || sg_raw_check.find("not found") != std::string::npos) {
            log("Warning: sg_raw not found. Install sg3-utils package for SuperDrive support.");
            log("  Ubuntu/Debian: sudo apt install sg3-utils");
            log("  Fedora/RHEL: sudo dnf install sg3_utils");
            log("  Arch: sudo pacman -S sg3_utils");
            return false;
        }
        
        std::stringstream cmd;
        cmd << "sg_raw " << shell_escape(device) << " EA 00 00 00 00 00 01 2>&1";
        std::string result = execute_command(cmd.str());
        
        if (result.find("Good") != std::string::npos) {
            log("SuperDrive wake-up successful");
            return true;
        }
        
        cmd.str("");
        cmd << "sg_raw --cmdset=1 " << shell_escape(device) << " EA 00 00 00 00 00 01 2>&1";
        result = execute_command(cmd.str());
        
        if (result.find("Good") != std::string::npos) {
            log("SuperDrive wake-up successful (with cmdset=1)");
            return true;
        }
        
        log("SuperDrive wake-up attempted: " + result);
        return true;
    }

    std::string getMacOSOpticalDiskDevice() {
        std::string result = execute_command("diskutil list 2>/dev/null");
        std::istringstream iss(result);
        std::string line;
        
        while (std::getline(iss, line)) {
            if (line.find("/dev/disk") != std::string::npos) {
                std::regex disk_regex("/dev/disk(\\d+)");
                std::smatch match;
                if (std::regex_search(line, match, disk_regex)) {
                    std::string disk_num = match[1].str();
                    std::string info = execute_command("diskutil info /dev/disk" + disk_num + " 2>/dev/null");
                    
                    if (info.find("Optical") != std::string::npos || 
                        info.find("CD") != std::string::npos || 
                        info.find("DVD") != std::string::npos) {
                        return "/dev/disk" + disk_num;
                    }
                }
            }
        }
        
        std::string drutil_status = execute_command("drutil status 2>/dev/null");
        if (drutil_status.find("No Media") == std::string::npos &&
            drutil_status.find("No optical drive") == std::string::npos) {
            
            result = execute_command("diskutil list | grep -E '^/dev/disk[0-9]+' | head -5 2>/dev/null");
            std::istringstream disk_iss(result);
            while (std::getline(disk_iss, line)) {
                std::regex disk_regex("/dev/disk(\\d+)");
                std::smatch match;
                if (std::regex_search(line, match, disk_regex)) {
                    std::string disk = "/dev/disk" + match[1].str();
                    std::string info = execute_command("diskutil info " + disk + " 2>/dev/null");
                    if (info.find("external") != std::string::npos || 
                        info.find("Removable") != std::string::npos) {
                        return disk;
                    }
                }
            }
        }
        
        return "";
    }

public:
    BurnCMECore() : output_directory("./extracted_data") {
        platform = detectPlatform();
        
        if (!fs::exists(output_directory)) {
            fs::create_directories(output_directory);
        }
        
        std::string os_name = (platform == Platform::macOS) ? "macOS" : 
                              (platform == Platform::Linux) ? "Linux" : "Unknown";
        log("Burn-CME initialized on " + os_name);
    }

    Platform getPlatform() const { return platform; }
    
    void setLogCallback(LogCallback cb) { log_callback = cb; }
    void setProgressCallback(ProgressCallback cb) { progress_callback = cb; }

    std::vector<DriveInfo> detectDrives() {
        std::vector<DriveInfo> drives;
        
        if (platform == Platform::macOS) {
            std::string status = execute_command("drutil status 2>/dev/null");
            
            if (status.find("No optical drive") == std::string::npos) {
                DriveInfo info;
                info.path = "default";
                info.name = "Optical Drive";
                
                std::string drutil_info = execute_command("drutil info 2>&1");
                if (drutil_info.find("Vendor") != std::string::npos) {
                    std::istringstream iss(drutil_info);
                    std::string line;
                    while (std::getline(iss, line)) {
                        if (line.find("Vendor") != std::string::npos || 
                            line.find("Product") != std::string::npos) {
                            info.name = line;
                            break;
                        }
                    }
                }
                
                info.capabilities = status;
                drives.push_back(info);
            }
        } else if (platform == Platform::Linux) {
            std::string result = execute_command("ls /dev/sr* /dev/cdrom* /dev/dvd* 2>/dev/null");
            std::istringstream iss(result);
            std::string line;
            std::set<std::string> seen;
            
            while (std::getline(iss, line)) {
                if (!line.empty() && line[0] == '/') {
                    try {
                        std::string real_path = line;
                        if (fs::is_symlink(line)) {
                            real_path = fs::read_symlink(line).string();
                            if (real_path[0] != '/') {
                                real_path = fs::path(line).parent_path() / real_path;
                            }
                        }
                        
                        if (seen.count(real_path)) continue;
                        seen.insert(real_path);
                        
                        DriveInfo info;
                        info.path = line;
                        info.name = fs::path(line).filename().string();
                        info.is_apple_superdrive = isAppleSuperDriveDevice(line);
                        
                        if (info.is_apple_superdrive) {
                            info.name += " (Apple SuperDrive)";
                        }
                        
                        drives.push_back(info);
                    } catch (...) {
                        DriveInfo info;
                        info.path = line;
                        info.name = fs::path(line).filename().string();
                        drives.push_back(info);
                    }
                }
            }
        }
        
        return drives;
    }

    void setCurrentDrive(const std::string& drive) {
        current_drive = drive;
        log("Selected drive: " + drive);
        
        if (platform == Platform::Linux && !drive.empty()) {
            current_drive_is_superdrive = isAppleSuperDriveDevice(drive);
            if (current_drive_is_superdrive) {
                wakeAppleSuperDrive(drive);
            }
        } else {
            current_drive_is_superdrive = false;
        }
    }

    std::string getCurrentDrive() const { return current_drive; }

    bool addPath(const std::string& path) {
        if (!fs::exists(path)) {
            log("File not found: " + path);
            return false;
        }

        fs::path abs_path = fs::absolute(path);
        paths_added.push_back(abs_path.string());

        if (fs::is_directory(abs_path)) {
            int count = 0;
            for (const auto& entry : fs::recursive_directory_iterator(abs_path)) {
                if (fs::is_regular_file(entry)) {
                    FileEntry fe;
                    fe.path = entry.path().string();
                    fe.display_name = entry.path().filename().string();
                    fe.size = fs::file_size(entry);
                    fe.is_directory = false;
                    file_queue.push_back(fe);
                    count++;
                }
            }
            log("Added directory: " + abs_path.string() + " (" + std::to_string(count) + " files)");
        } else {
            FileEntry fe;
            fe.path = abs_path.string();
            fe.display_name = abs_path.filename().string();
            fe.size = fs::file_size(abs_path);
            fe.is_directory = false;
            file_queue.push_back(fe);
            log("Added file: " + abs_path.string());
        }
        return true;
    }

    const std::vector<FileEntry>& getFileQueue() const { return file_queue; }
    const std::vector<std::string>& getPathsAdded() const { return paths_added; }

    size_t getTotalSize() const {
        size_t total = 0;
        for (const auto& fe : file_queue) {
            total += fe.size;
        }
        return total;
    }

    void clearQueue() {
        file_queue.clear();
        paths_added.clear();
        log("File queue cleared");
    }

    bool createISO(const std::string& iso_name) {
        if (paths_added.empty()) {
            log("No files in queue");
            return false;
        }

        std::string iso_path = iso_name;
        if (iso_path.find(".iso") == std::string::npos) {
            iso_path += ".iso";
        }

        std::string staging_dir = "/tmp/burn_staging_" + std::to_string(getpid());
        fs::create_directories(staging_dir);

        log("Copying files to staging area...");
        std::set<std::string> used_names;

        for (const auto& added_path : paths_added) {
            try {
                fs::path src_path = fs::path(added_path).lexically_normal();
                while (src_path.has_filename() && src_path.filename().empty()) {
                    src_path = src_path.parent_path();
                }
                std::string base_name = src_path.filename().string();
                if (base_name.empty()) base_name = "root";
                
                std::string unique_name = generate_unique_name(base_name, used_names);
                used_names.insert(unique_name);

                fs::path dest_base = fs::path(staging_dir) / unique_name;

                if (fs::is_directory(src_path)) {
                    fs::copy(src_path, dest_base, fs::copy_options::recursive);
                    log("Copied directory: " + base_name);
                } else if (fs::is_regular_file(src_path)) {
                    fs::copy_file(src_path, dest_base);
                    log("Copied: " + base_name);
                }
            } catch (const std::exception& e) {
                log("Error copying: " + std::string(e.what()));
            }
        }

        log("Creating ISO image: " + iso_path);

        if (platform == Platform::macOS) {
            std::stringstream cmd;
            cmd << "hdiutil makehybrid -o " << shell_escape(iso_path) 
                << " -iso -joliet -default-volume-name BURN_CME " 
                << shell_escape(staging_dir) << " 2>&1";
            
            std::string result = execute_command(cmd.str());
            log(result);
            
            if (!fs::exists(iso_path)) {
                log("Trying with mkisofs fallback...");
                std::stringstream alt_cmd;
                alt_cmd << "mkisofs -r -J -V 'BURN_CME' -o " 
                        << shell_escape(iso_path) << " " << shell_escape(staging_dir) << " 2>&1";
                result = execute_command(alt_cmd.str());
                log(result);
            }
        } else {
            std::stringstream cmd;
            cmd << "mkisofs -r -J -joliet-long -V 'BURN_CME' -o " 
                << shell_escape(iso_path) << " " << shell_escape(staging_dir) << " 2>&1";

            std::string result = execute_command(cmd.str());
            log(result);

            if (!fs::exists(iso_path)) {
                log("Trying alternative method with xorrisofs...");
                std::stringstream alt_cmd;
                alt_cmd << "xorrisofs -r -J -V 'BURN_CME' -o " 
                        << shell_escape(iso_path) << " " << shell_escape(staging_dir) << " 2>&1";
                result = execute_command(alt_cmd.str());
                log(result);
            }
        }

        fs::remove_all(staging_dir);

        if (fs::exists(iso_path)) {
            auto size = fs::file_size(iso_path);
            log("ISO created successfully: " + iso_path + " (" + 
                std::to_string(size / 1024 / 1024) + " MB)");
            return true;
        }
        
        log("Failed to create ISO");
        return false;
    }

    bool burnDisc(const std::string& iso_path, int speed = 0) {
        if (!fs::exists(iso_path)) {
            log("ISO file not found: " + iso_path);
            return false;
        }

        log("Starting burn process...");

        if (platform == Platform::macOS) {
            std::stringstream cmd;
            cmd << "hdiutil burn " << shell_escape(iso_path);
            if (speed > 0) {
                cmd << " -speed " << speed;
            }
            cmd << " 2>&1";
            
            std::string result = execute_command(cmd.str());
            log(result);
            
            if (result.find("Burn completed successfully") != std::string::npos ||
                result.find("Burn complete") != std::string::npos) {
                log("Burn completed successfully");
                return true;
            } else if (result.find("Error") != std::string::npos ||
                       result.find("failed") != std::string::npos) {
                log("Burn failed");
                return false;
            }
            return true;
        } else {
            std::string drive = current_drive.empty() ? "/dev/sr0" : current_drive;
            
            if (current_drive_is_superdrive) {
                wakeAppleSuperDrive(drive);
            }
            
            std::stringstream cmd;
            cmd << "cdrecord -v speed=" << speed << " dev=" 
                << shell_escape(drive) << " " << shell_escape(iso_path) << " 2>&1";

            std::string result = execute_command(cmd.str());
            log(result);

            bool has_error = result.find("Error") != std::string::npos || 
                            result.find("Cannot") != std::string::npos;
            
            if (has_error) {
                log("Trying with wodim...");
                std::stringstream alt_cmd;
                alt_cmd << "wodim -v speed=" << speed << " dev=" 
                        << shell_escape(drive) << " " << shell_escape(iso_path) << " 2>&1";
                result = execute_command(alt_cmd.str());
                log(result);
                
                has_error = result.find("Error") != std::string::npos || 
                           result.find("Cannot") != std::string::npos;
            }
            
            if (has_error) {
                log("Burn may have failed - check output above");
                return false;
            }
        }

        log("Burn process completed");
        return true;
    }

    bool extractToISO(const std::string& output_path) {
        log("Creating ISO image from disc...");

        if (platform == Platform::macOS) {
            std::string status = execute_command("drutil status 2>&1");
            log("Drive status: " + status);
            
            if (status.find("No Media") != std::string::npos) {
                log("No disc inserted. Please insert a CD/DVD.");
                return false;
            }
            
            if (status.find("No optical drive") != std::string::npos) {
                log("No optical drive found.");
                return false;
            }
            
            std::string disk_device = getMacOSOpticalDiskDevice();
            
            std::string mount_point;
            std::string volumes_output = execute_command("ls /Volumes 2>/dev/null");
            std::istringstream vol_stream(volumes_output);
            std::string vol_line;
            while (std::getline(vol_stream, vol_line)) {
                if (!vol_line.empty() && vol_line.find("Macintosh") == std::string::npos) {
                    mount_point = "/Volumes/" + vol_line;
                    break;
                }
            }
            
            if (mount_point.empty() && disk_device.empty()) {
                log("Could not find mounted disc or optical device.");
                return false;
            }
            
            std::string temp_path = output_path;
            if (temp_path.size() > 4 && temp_path.substr(temp_path.size() - 4) == ".iso") {
                temp_path = temp_path.substr(0, temp_path.size() - 4);
            }
            
            std::stringstream cmd;
            if (!mount_point.empty() && fs::exists(mount_point)) {
                log("Creating ISO from mounted volume: " + mount_point);
                cmd << "hdiutil makehybrid -iso -joliet -o " << shell_escape(temp_path + ".iso")
                    << " " << shell_escape(mount_point) << " 2>&1";
            } else if (!disk_device.empty()) {
                log("Found optical device: " + disk_device);
                cmd << "dd if=" << shell_escape(disk_device) 
                    << " of=" << shell_escape(output_path) << " bs=2048 2>&1";
            }
            
            std::string result = execute_command(cmd.str());
            log(result);
        } else {
            std::string drive = current_drive.empty() ? "/dev/sr0" : current_drive;
            
            if (current_drive_is_superdrive) {
                wakeAppleSuperDrive(drive);
            }
            
            std::stringstream cmd;
            cmd << "dd if=" << shell_escape(drive) << " of=" 
                << shell_escape(output_path) << " bs=2048 status=progress 2>&1";

            std::string result = execute_command(cmd.str());
            log(result);
        }

        if (fs::exists(output_path) && fs::file_size(output_path) > 0) {
            auto size = fs::file_size(output_path);
            log("ISO extracted: " + output_path + " (" + 
                std::to_string(size / 1024 / 1024) + " MB)");
            return true;
        }

        log("Failed to extract ISO");
        return false;
    }

    std::string getDiscInfo() {
        std::stringstream info;
        
        if (platform == Platform::macOS) {
            info << "=== macOS Optical Drive Information ===\n\n";
            
            std::string status = execute_command("drutil status 2>&1");
            info << "Drive Status:\n" << status << "\n";
            
            std::string driveInfo = execute_command("drutil info 2>&1");
            info << "Drive Info:\n" << driveInfo << "\n";
            
            std::string disk = getMacOSOpticalDiskDevice();
            if (!disk.empty()) {
                info << "Optical Device: " << disk << "\n";
                std::string diskInfo = execute_command("diskutil info " + disk + " 2>&1");
                info << "\nDisk Info:\n" << diskInfo << "\n";
            }
        } else {
            std::string drive = current_drive.empty() ? "/dev/sr0" : current_drive;
            
            if (current_drive_is_superdrive) {
                wakeAppleSuperDrive(drive);
            }
            
            info << "Drive: " << drive << "\n\n";

            std::stringstream cmd;
            cmd << "cdrecord dev=" << shell_escape(drive) << " -checkdrive 2>&1";
            info << "Drive capabilities:\n" << execute_command(cmd.str()) << "\n";

            cmd.str("");
            cmd << "cdrecord dev=" << shell_escape(drive) << " -atip 2>&1";
            info << "Media info:\n" << execute_command(cmd.str()) << "\n";
            
            if (current_drive_is_superdrive) {
                info << "\n=== Apple SuperDrive ===\n";
                info << "This drive requires wake-up on Linux.\n";
                info << "Wake-up command: sg_raw " << drive << " EA 00 00 00 00 00 01\n";
            }
        }

        return info.str();
    }

    bool eraseDisc(bool full = false) {
        log("Erasing disc...");

        if (platform == Platform::macOS) {
            std::string erase_type = full ? "full" : "quick";
            std::stringstream cmd;
            cmd << "drutil erase " << erase_type << " 2>&1";
            
            std::string result = execute_command(cmd.str());
            log(result);
            
            if (result.find("error") != std::string::npos ||
                result.find("Error") != std::string::npos ||
                result.find("failed") != std::string::npos) {
                log("Erase failed");
                return false;
            }
            
            log("Erase completed");
            return true;
        } else {
            std::string drive = current_drive.empty() ? "/dev/sr0" : current_drive;
            
            if (current_drive_is_superdrive) {
                wakeAppleSuperDrive(drive);
            }
            
            std::string blank_type = full ? "blank=all" : "blank=fast";
            log("Erasing disc (" + blank_type + ")...");

            std::stringstream cmd;
            cmd << "cdrecord dev=" << shell_escape(drive) << " " << blank_type << " 2>&1";

            std::string result = execute_command(cmd.str());
            log(result);
            
            if (result.find("Error") != std::string::npos ||
                result.find("Cannot") != std::string::npos) {
                log("Erase may have failed");
                return false;
            }
            
            log("Erase completed");
            return true;
        }
    }

    bool ejectDisc() {
        log("Ejecting disc...");
        
        if (platform == Platform::macOS) {
            std::string result = execute_command("drutil tray eject 2>&1");
            log(result);
        } else {
            std::string drive = current_drive.empty() ? "/dev/sr0" : current_drive;
            std::stringstream cmd;
            cmd << "eject " << shell_escape(drive) << " 2>&1";
            std::string result = execute_command(cmd.str());
            log(result);
        }
        
        log("Eject command sent");
        return true;
    }

    void setOutputDirectory(const std::string& dir) {
        output_directory = dir;
        if (!fs::exists(output_directory)) {
            fs::create_directories(output_directory);
        }
    }

    std::string getOutputDirectory() const { return output_directory; }

    bool checkFFmpeg() {
        std::string result = execute_command("which ffmpeg 2>/dev/null || command -v ffmpeg 2>/dev/null");
        return !result.empty() && result.find("not found") == std::string::npos;
    }

    bool checkDvdauthor() {
        std::string result = execute_command("which dvdauthor 2>/dev/null || command -v dvdauthor 2>/dev/null");
        return !result.empty() && result.find("not found") == std::string::npos;
    }

    bool convertMP4toDVDVideo(const std::string& input_mp4, const std::string& output_dir) {
        if (!checkFFmpeg()) {
            log("Error: FFmpeg is not installed. Please install FFmpeg to use video conversion.");
            return false;
        }

        if (!fs::exists(input_mp4)) {
            log("Error: Input file does not exist: " + input_mp4);
            return false;
        }

        fs::path out_path(output_dir);
        fs::path video_ts = out_path / "VIDEO_TS";
        
        if (!fs::exists(video_ts)) {
            fs::create_directories(video_ts);
        }

        log("Converting MP4 to DVD-Video format...");
        log("Input: " + input_mp4);
        log("Output: " + output_dir);

        fs::path mpeg_file = out_path / "video.mpg";
        
        std::stringstream cmd;
        cmd << "ffmpeg -y -i " << shell_escape(input_mp4)
            << " -target ntsc-dvd"
            << " -aspect 16:9"
            << " -b:v 6000k"
            << " -maxrate 9000k"
            << " -bufsize 1835k"
            << " -muxrate 10080k"
            << " -b:a 448k"
            << " -ar 48000"
            << " -ac 2"
            << " " << shell_escape(mpeg_file.string())
            << " 2>&1";

        log("Running FFmpeg conversion...");
        std::string result = execute_command(cmd.str());
        
        if (!fs::exists(mpeg_file)) {
            log("Error: FFmpeg conversion failed");
            log(result);
            return false;
        }

        log("MPEG-2 file created successfully");

        if (checkDvdauthor()) {
            log("Creating DVD structure with dvdauthor...");
            
            fs::path dvd_xml = out_path / "dvd.xml";
            std::ofstream xml_file(dvd_xml);
            xml_file << "<dvdauthor dest=\"" << video_ts.parent_path().string() << "\">\n";
            xml_file << "  <vmgm />\n";
            xml_file << "  <titleset>\n";
            xml_file << "    <titles>\n";
            xml_file << "      <pgc>\n";
            xml_file << "        <vob file=\"" << mpeg_file.string() << "\" />\n";
            xml_file << "      </pgc>\n";
            xml_file << "    </titles>\n";
            xml_file << "  </titleset>\n";
            xml_file << "</dvdauthor>\n";
            xml_file.close();

            std::stringstream dvd_cmd;
            dvd_cmd << "VIDEO_FORMAT=NTSC dvdauthor -x " << shell_escape(dvd_xml.string()) << " 2>&1";
            result = execute_command(dvd_cmd.str());
            log(result);

            fs::remove(dvd_xml);
            log("DVD structure created in VIDEO_TS folder");
        } else {
            log("Note: dvdauthor not installed. Created MPEG-2 file only.");
            log("For full DVD structure, install dvdauthor: sudo apt install dvdauthor");
        }

        log("Video DVD conversion completed!");
        return true;
    }

    bool convertMultipleMP4stoDVD(const std::vector<std::string>& input_files, const std::string& output_dir) {
        if (!checkFFmpeg()) {
            log("Error: FFmpeg is not installed.");
            return false;
        }

        fs::path out_path(output_dir);
        fs::path video_ts = out_path / "VIDEO_TS";
        
        if (!fs::exists(video_ts)) {
            fs::create_directories(video_ts);
        }

        log("Converting " + std::to_string(input_files.size()) + " video files to DVD format...");

        std::vector<std::string> mpeg_files;
        int file_num = 1;
        
        for (const auto& input_mp4 : input_files) {
            if (!fs::exists(input_mp4)) {
                log("Warning: Skipping non-existent file: " + input_mp4);
                continue;
            }

            fs::path mpeg_file = out_path / ("video_" + std::to_string(file_num++) + ".mpg");
            
            log("Converting: " + input_mp4);
            
            std::stringstream cmd;
            cmd << "ffmpeg -y -i " << shell_escape(input_mp4)
                << " -target ntsc-dvd"
                << " -aspect 16:9"
                << " -b:v 6000k"
                << " -maxrate 9000k"
                << " -bufsize 1835k"
                << " " << shell_escape(mpeg_file.string())
                << " 2>&1";

            std::string result = execute_command(cmd.str());
            
            if (fs::exists(mpeg_file)) {
                mpeg_files.push_back(mpeg_file.string());
                log("Converted successfully: " + mpeg_file.string());
            } else {
                log("Warning: Failed to convert: " + input_mp4);
            }
        }

        if (mpeg_files.empty()) {
            log("Error: No files were converted successfully");
            return false;
        }

        if (checkDvdauthor()) {
            log("Creating DVD structure...");
            
            fs::path dvd_xml = out_path / "dvd.xml";
            std::ofstream xml_file(dvd_xml);
            xml_file << "<dvdauthor dest=\"" << video_ts.parent_path().string() << "\">\n";
            xml_file << "  <vmgm />\n";
            xml_file << "  <titleset>\n";
            xml_file << "    <titles>\n";
            xml_file << "      <pgc>\n";
            for (const auto& mpg : mpeg_files) {
                xml_file << "        <vob file=\"" << mpg << "\" />\n";
            }
            xml_file << "      </pgc>\n";
            xml_file << "    </titles>\n";
            xml_file << "  </titleset>\n";
            xml_file << "</dvdauthor>\n";
            xml_file.close();

            std::stringstream dvd_cmd;
            dvd_cmd << "VIDEO_FORMAT=NTSC dvdauthor -x " << shell_escape(dvd_xml.string()) << " 2>&1";
            std::string result = execute_command(dvd_cmd.str());
            log(result);
            
            fs::remove(dvd_xml);
        }

        log("Video DVD creation completed!");
        return true;
    }

    bool extractDVDtoMP4(const std::string& dvd_path, const std::string& output_mp4) {
        if (!checkFFmpeg()) {
            log("Error: FFmpeg is not installed.");
            return false;
        }

        log("Extracting DVD video to MP4...");
        log("Source: " + dvd_path);
        log("Output: " + output_mp4);

        fs::path dvd(dvd_path);
        std::string input_source;

        if (fs::is_directory(dvd)) {
            fs::path video_ts = dvd / "VIDEO_TS";
            if (fs::exists(video_ts)) {
                std::vector<std::string> vob_files;
                for (const auto& entry : fs::directory_iterator(video_ts)) {
                    std::string filename = entry.path().filename().string();
                    if (filename.find("VTS_") == 0 && filename.find("_1.VOB") != std::string::npos) {
                        vob_files.push_back(entry.path().string());
                    }
                }
                
                if (vob_files.empty()) {
                    for (const auto& entry : fs::directory_iterator(video_ts)) {
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);
                        if (ext == ".VOB" && entry.path().filename().string() != "VIDEO_TS.VOB") {
                            vob_files.push_back(entry.path().string());
                        }
                    }
                }
                
                if (!vob_files.empty()) {
                    std::sort(vob_files.begin(), vob_files.end());
                    input_source = "concat:";
                    for (size_t i = 0; i < vob_files.size(); i++) {
                        if (i > 0) input_source += "|";
                        input_source += vob_files[i];
                    }
                    log("Found " + std::to_string(vob_files.size()) + " VOB files");
                }
            }
        } else if (fs::is_block_file(dvd) || dvd_path.find("/dev/") == 0) {
            input_source = dvd_path;
            log("Reading from DVD device: " + dvd_path);
        } else if (fs::exists(dvd)) {
            input_source = dvd_path;
        }

        if (input_source.empty()) {
            log("Error: Could not find video content in: " + dvd_path);
            return false;
        }

        std::stringstream cmd;
        cmd << "ffmpeg -y -i " << shell_escape(input_source)
            << " -c:v libx264"
            << " -preset medium"
            << " -crf 23"
            << " -c:a aac"
            << " -b:a 192k"
            << " -movflags +faststart"
            << " " << shell_escape(output_mp4)
            << " 2>&1";

        log("Converting to MP4 (this may take a while)...");
        std::string result = execute_command(cmd.str());
        
        if (fs::exists(output_mp4) && fs::file_size(output_mp4) > 0) {
            log("Successfully extracted to: " + output_mp4);
            return true;
        } else {
            log("Error: Conversion failed");
            log(result);
            return false;
        }
    }

    bool extractDVDDriveToMP4(const std::string& output_mp4) {
        std::string drive = current_drive.empty() ? "/dev/sr0" : current_drive;
        
        log("Accessing DVD drive...");
        
#ifdef __APPLE__
        // On macOS, DVDs are typically auto-mounted under /Volumes
        // Find the mounted DVD volume
        std::string mount_point;
        std::string volumes_output = execute_command("ls /Volumes 2>/dev/null");
        
        // Check if drive is a /dev/disk path, find its mount point
        if (drive.find("/dev/disk") != std::string::npos) {
            std::string mount_info = execute_command("diskutil info " + shell_escape(drive) + " 2>/dev/null | grep 'Mount Point'");
            size_t colon_pos = mount_info.find(':');
            if (colon_pos != std::string::npos) {
                mount_point = mount_info.substr(colon_pos + 1);
                // Trim whitespace
                mount_point.erase(0, mount_point.find_first_not_of(" \t\n\r"));
                mount_point.erase(mount_point.find_last_not_of(" \t\n\r") + 1);
            }
        }
        
        // If no mount point found, try common DVD volume names
        if (mount_point.empty() || !fs::exists(mount_point)) {
            for (const auto& entry : fs::directory_iterator("/Volumes")) {
                std::string vol_path = entry.path().string();
                if (fs::exists(vol_path + "/VIDEO_TS")) {
                    mount_point = vol_path;
                    break;
                }
            }
        }
        
        if (mount_point.empty() || !fs::exists(mount_point)) {
            log("Error: Could not find mounted DVD. Please ensure disc is inserted.");
            return false;
        }
        
        log("Found DVD at: " + mount_point);
        bool success = extractDVDtoMP4(mount_point, output_mp4);
        return success;
#else
        // Linux: mount the drive
        std::string mount_point = "/tmp/burn_cme_dvd_mount";
        fs::create_directories(mount_point);
        
        std::stringstream mount_cmd;
        mount_cmd << "mount -t udf,iso9660 " << shell_escape(drive) << " " << shell_escape(mount_point) << " 2>&1";
        
        std::string mount_result = execute_command(mount_cmd.str());
        
        bool success = extractDVDtoMP4(mount_point, output_mp4);
        
        execute_command("umount " + shell_escape(mount_point) + " 2>&1");
        
        return success;
#endif
    }

    std::vector<std::string> getVideoFilesFromQueue() {
        std::vector<std::string> video_files;
        std::vector<std::string> video_extensions = {".mp4", ".avi", ".mkv", ".mov", ".wmv", ".flv", ".webm", ".m4v", ".mpeg", ".mpg"};
        
        for (const auto& entry : file_queue) {
            if (!entry.is_directory) {
                std::string ext = fs::path(entry.path).extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                for (const auto& video_ext : video_extensions) {
                    if (ext == video_ext) {
                        video_files.push_back(entry.path);
                        break;
                    }
                }
            }
        }
        return video_files;
    }

    bool createVideoDVDFromQueue(const std::string& output_dir) {
        std::vector<std::string> video_files = getVideoFilesFromQueue();
        if (video_files.empty()) {
            log("Error: No video files in queue");
            return false;
        }
        return convertMultipleMP4stoDVD(video_files, output_dir);
    }

    bool burnVideoDVD(const std::string& video_dvd_dir) {
        fs::path video_ts = fs::path(video_dvd_dir) / "VIDEO_TS";
        if (!fs::exists(video_ts)) {
            log("Error: No VIDEO_TS folder found in: " + video_dvd_dir);
            return false;
        }

        log("Creating ISO from Video DVD structure...");
        
        std::string iso_path = video_dvd_dir + "/video_dvd.iso";
        
        std::stringstream mkisofs_cmd;
        mkisofs_cmd << "mkisofs -dvd-video -V 'VIDEO_DVD' -o "
                    << shell_escape(iso_path) << " "
                    << shell_escape(video_dvd_dir) << " 2>&1";
        
        std::string result = execute_command(mkisofs_cmd.str());
        log(result);
        
        if (!fs::exists(iso_path)) {
            log("Error: Failed to create Video DVD ISO");
            return false;
        }
        
        log("Video DVD ISO created, now burning...");
        return burnDisc(iso_path);
    }
};

#endif
