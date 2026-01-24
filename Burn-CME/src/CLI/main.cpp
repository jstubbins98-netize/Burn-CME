#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <memory>
#include <array>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <set>

namespace fs = std::filesystem;

class BurnCME {
private:
    std::vector<std::string> files_to_burn;
    std::vector<std::string> paths_added;
    std::string current_drive;
    std::string output_directory;
    
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

public:
    BurnCME() : output_directory("./extracted_data") {
        if (!fs::exists(output_directory)) {
            fs::create_directories(output_directory);
        }
    }
    
    void display_header() {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                      BURN-CME v1.0                           ║\n";
        std::cout << "║           CD/DVD Burning and Extraction Utility              ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
        std::cout << "\n";
    }
    
    void display_menu() {
        std::cout << "\n┌─────────────────────────────────────┐\n";
        std::cout << "│          MAIN MENU                  │\n";
        std::cout << "├─────────────────────────────────────┤\n";
        std::cout << "│  1. Detect CD/DVD Drives            │\n";
        std::cout << "│  2. Add Files to Burn               │\n";
        std::cout << "│  3. View Files Queue                │\n";
        std::cout << "│  4. Clear Files Queue               │\n";
        std::cout << "│  5. Create ISO Image                │\n";
        std::cout << "│  6. Burn to CD/DVD                  │\n";
        std::cout << "│  7. Extract Data from CD/DVD        │\n";
        std::cout << "│  8. View Disc Info                  │\n";
        std::cout << "│  9. Erase Rewritable Disc           │\n";
        std::cout << "│  A. Create Video DVD from MP4       │\n";
        std::cout << "│  B. Extract DVD Video to MP4        │\n";
        std::cout << "│  0. Exit                            │\n";
        std::cout << "└─────────────────────────────────────┘\n";
        std::cout << "\nEnter your choice: ";
    }
    
    void detect_drives() {
        std::cout << "\n[*] Scanning for CD/DVD drives...\n\n";
        
        std::string result = execute_command("ls -la /dev/sr* /dev/cdrom* /dev/dvd* 2>/dev/null || echo 'No optical drives detected'");
        std::cout << "Detected devices:\n" << result << "\n";
        
        std::cout << "\n[*] Checking drive capabilities using cdrecord...\n";
        std::string cdrecord_result = execute_command("cdrecord -scanbus 2>&1 || wodim -scanbus 2>&1 || echo 'Could not scan bus'");
        std::cout << cdrecord_result << "\n";
        
        std::cout << "Enter drive path (e.g., /dev/sr0) or press Enter to skip: ";
        std::string drive;
        std::getline(std::cin, drive);
        if (!drive.empty()) {
            current_drive = drive;
            std::cout << "[+] Selected drive: " << current_drive << "\n";
        }
    }
    
    void add_files() {
        std::cout << "\n[*] Add files to burn queue\n";
        std::cout << "Enter file or directory path (or 'done' to finish): ";
        
        std::string path;
        while (std::getline(std::cin, path) && path != "done") {
            if (path.empty()) {
                std::cout << "Enter path (or 'done'): ";
                continue;
            }
            
            if (fs::exists(path)) {
                fs::path abs_path = fs::absolute(path);
                paths_added.push_back(abs_path.string());
                
                if (fs::is_directory(abs_path)) {
                    int file_count = 0;
                    for (const auto& entry : fs::recursive_directory_iterator(abs_path)) {
                        if (fs::is_regular_file(entry)) {
                            files_to_burn.push_back(entry.path().string());
                            file_count++;
                        }
                    }
                    std::cout << "[+] Added directory: " << abs_path.string() << " (" << file_count << " files)\n";
                } else {
                    files_to_burn.push_back(abs_path.string());
                    std::cout << "[+] Added: " << abs_path.string() << "\n";
                }
            } else {
                std::cout << "[-] File not found: " << path << "\n";
            }
            std::cout << "Enter path (or 'done'): ";
        }
        
        std::cout << "\n[*] Total files in queue: " << files_to_burn.size() << "\n";
    }
    
    void view_queue() {
        std::cout << "\n┌─────────────────────────────────────┐\n";
        std::cout << "│        FILES IN BURN QUEUE          │\n";
        std::cout << "└─────────────────────────────────────┘\n";
        
        if (files_to_burn.empty()) {
            std::cout << "\n(Queue is empty)\n";
            return;
        }
        
        size_t total_size = 0;
        for (size_t i = 0; i < files_to_burn.size(); ++i) {
            try {
                auto size = fs::file_size(files_to_burn[i]);
                total_size += size;
                std::cout << "  " << (i + 1) << ". " << files_to_burn[i];
                std::cout << " (" << (size / 1024.0 / 1024.0) << " MB)\n";
            } catch (...) {
                std::cout << "  " << (i + 1) << ". " << files_to_burn[i] << " (size unknown)\n";
            }
        }
        
        std::cout << "\n[*] Total: " << files_to_burn.size() << " files";
        std::cout << " (" << (total_size / 1024.0 / 1024.0) << " MB)\n";
        
        double cd_capacity = 700.0;
        double dvd_capacity = 4700.0;
        double total_mb = total_size / 1024.0 / 1024.0;
        
        std::cout << "\n[*] Disc requirements:\n";
        if (total_mb <= cd_capacity) {
            std::cout << "    - Fits on a standard CD (700 MB)\n";
        } else if (total_mb <= dvd_capacity) {
            std::cout << "    - Requires DVD (4.7 GB)\n";
        } else {
            std::cout << "    - Requires dual-layer DVD or multiple discs\n";
        }
    }
    
    void clear_queue() {
        files_to_burn.clear();
        paths_added.clear();
        std::cout << "\n[+] File queue cleared.\n";
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
    
    void create_iso() {
        if (paths_added.empty()) {
            std::cout << "\n[-] No files in queue. Add files first.\n";
            return;
        }
        
        std::cout << "\n[*] Create ISO Image\n";
        std::cout << "Enter ISO filename (without extension): ";
        std::string iso_name;
        std::getline(std::cin, iso_name);
        
        if (iso_name.empty()) {
            iso_name = "burn_image";
        }
        
        std::string iso_path = iso_name + ".iso";
        
        std::string staging_dir = "/tmp/burn_staging_" + std::to_string(getpid());
        fs::create_directories(staging_dir);
        
        std::cout << "\n[*] Copying files to staging area (preserving structure)...\n";
        
        std::set<std::string> used_names;
        
        for (const auto& added_path : paths_added) {
            try {
                fs::path src_path = fs::path(added_path).lexically_normal();
                while (src_path.has_filename() && src_path.filename().empty()) {
                    src_path = src_path.parent_path();
                }
                std::string base_name = src_path.filename().string();
                if (base_name.empty()) {
                    base_name = "root";
                }
                std::string unique_name = generate_unique_name(base_name, used_names);
                used_names.insert(unique_name);
                
                fs::path dest_base = fs::path(staging_dir) / unique_name;
                
                if (fs::is_directory(src_path)) {
                    fs::copy(src_path, dest_base, 
                             fs::copy_options::recursive);
                    std::cout << "    Copied directory: " << base_name;
                    if (unique_name != base_name) {
                        std::cout << " -> " << unique_name;
                    }
                    std::cout << "/\n";
                } else if (fs::is_regular_file(src_path)) {
                    fs::copy_file(src_path, dest_base);
                    std::cout << "    Copied: " << base_name;
                    if (unique_name != base_name) {
                        std::cout << " -> " << unique_name;
                    }
                    std::cout << "\n";
                }
            } catch (const std::exception& e) {
                std::cout << "    [-] Error copying " << added_path << ": " << e.what() << "\n";
            }
        }
        
        std::cout << "\n[*] Creating ISO image: " << iso_path << "\n";
        
        std::stringstream cmd;
        cmd << "mkisofs -r -J -joliet-long -V 'BURN_CME' -o " << shell_escape(iso_path) << " " << shell_escape(staging_dir) << " 2>&1";
        
        std::string result = execute_command(cmd.str());
        std::cout << result << "\n";
        
        if (fs::exists(iso_path)) {
            auto size = fs::file_size(iso_path);
            std::cout << "\n[+] ISO created successfully: " << iso_path;
            std::cout << " (" << (size / 1024.0 / 1024.0) << " MB)\n";
        } else {
            std::cout << "\n[-] Failed to create ISO image.\n";
            std::cout << "[*] Trying alternative method with xorrisofs...\n";
            
            std::stringstream alt_cmd;
            alt_cmd << "xorrisofs -r -J -V 'BURN_CME' -o " << shell_escape(iso_path) << " " << shell_escape(staging_dir) << " 2>&1";
            result = execute_command(alt_cmd.str());
            std::cout << result << "\n";
            
            if (fs::exists(iso_path)) {
                auto size = fs::file_size(iso_path);
                std::cout << "\n[+] ISO created successfully: " << iso_path;
                std::cout << " (" << (size / 1024.0 / 1024.0) << " MB)\n";
            }
        }
        
        fs::remove_all(staging_dir);
    }
    
    void burn_disc() {
        if (current_drive.empty()) {
            std::cout << "\n[-] No drive selected. Please detect drives first (option 1).\n";
            return;
        }
        
        std::cout << "\n[*] Burn to CD/DVD\n";
        std::cout << "Enter ISO file path to burn: ";
        std::string iso_path;
        std::getline(std::cin, iso_path);
        
        if (!fs::exists(iso_path)) {
            std::cout << "[-] File not found: " << iso_path << "\n";
            return;
        }
        
        std::cout << "\n[*] Burn speed options:\n";
        std::cout << "    1. Auto (recommended)\n";
        std::cout << "    2. 4x\n";
        std::cout << "    3. 8x\n";
        std::cout << "    4. 16x\n";
        std::cout << "    5. Maximum\n";
        std::cout << "Select speed: ";
        
        std::string speed_choice;
        std::getline(std::cin, speed_choice);
        
        std::string speed = "speed=0";
        if (speed_choice == "2") speed = "speed=4";
        else if (speed_choice == "3") speed = "speed=8";
        else if (speed_choice == "4") speed = "speed=16";
        else if (speed_choice == "5") speed = "speed=0";
        
        std::cout << "\n[!] WARNING: This will write data to " << current_drive << "\n";
        std::cout << "    Make sure a blank/rewritable disc is inserted.\n";
        std::cout << "    Continue? (yes/no): ";
        
        std::string confirm;
        std::getline(std::cin, confirm);
        
        if (confirm != "yes") {
            std::cout << "[-] Burn cancelled.\n";
            return;
        }
        
        std::cout << "\n[*] Starting burn process...\n";
        std::cout << "[*] Using cdrecord/wodim to burn disc...\n\n";
        
        std::stringstream cmd;
        cmd << "cdrecord -v " << speed << " dev=" << shell_escape(current_drive) << " " << shell_escape(iso_path) << " 2>&1";
        
        std::string result = execute_command(cmd.str());
        std::cout << result << "\n";
        
        if (result.find("Error") != std::string::npos || result.find("error") != std::string::npos) {
            std::cout << "\n[*] Trying with wodim...\n";
            std::stringstream alt_cmd;
            alt_cmd << "wodim -v " << speed << " dev=" << shell_escape(current_drive) << " " << shell_escape(iso_path) << " 2>&1";
            result = execute_command(alt_cmd.str());
            std::cout << result << "\n";
        }
        
        std::cout << "\n[*] Burn process completed.\n";
    }
    
    void extract_data() {
        if (current_drive.empty()) {
            std::cout << "\n[*] No drive selected. Enter drive path (e.g., /dev/sr0): ";
            std::getline(std::cin, current_drive);
            if (current_drive.empty()) {
                current_drive = "/dev/sr0";
            }
        }
        
        std::cout << "\n[*] Extract Data from CD/DVD\n";
        std::cout << "Output directory [" << output_directory << "]: ";
        std::string out_dir;
        std::getline(std::cin, out_dir);
        
        if (!out_dir.empty()) {
            output_directory = out_dir;
        }
        
        if (!fs::exists(output_directory)) {
            fs::create_directories(output_directory);
        }
        
        std::cout << "\n[*] Extraction options:\n";
        std::cout << "    1. Copy all files from mounted disc\n";
        std::cout << "    2. Create ISO image from disc\n";
        std::cout << "Select option: ";
        
        std::string option;
        std::getline(std::cin, option);
        
        if (option == "1") {
            std::string mount_point;
            std::string result;
            
#ifdef __APPLE__
            // macOS: Find the auto-mounted disc volume
            std::cout << "\n[*] Finding mounted disc...\n";
            
            // Try to get mount point from diskutil
            if (current_drive.find("/dev/disk") != std::string::npos) {
                std::string mount_info = execute_command("diskutil info " + shell_escape(current_drive) + " 2>/dev/null | grep 'Mount Point'");
                size_t colon_pos = mount_info.find(':');
                if (colon_pos != std::string::npos) {
                    mount_point = mount_info.substr(colon_pos + 1);
                    mount_point.erase(0, mount_point.find_first_not_of(" \t\n\r"));
                    mount_point.erase(mount_point.find_last_not_of(" \t\n\r") + 1);
                }
            }
            
            // Search /Volumes for optical discs if not found
            if (mount_point.empty() || !fs::exists(mount_point)) {
                for (const auto& entry : fs::directory_iterator("/Volumes")) {
                    std::string vol_path = entry.path().string();
                    // Skip system volumes
                    if (vol_path != "/Volumes/Macintosh HD" && vol_path != "/Volumes/Preboot" && 
                        vol_path != "/Volumes/Recovery" && vol_path != "/Volumes/Data") {
                        mount_point = vol_path;
                        break;
                    }
                }
            }
            
            if (mount_point.empty() || !fs::exists(mount_point)) {
                std::cout << "[-] Could not find mounted disc. Please ensure disc is inserted.\n";
                return;
            }
            
            std::cout << "[+] Found disc at: " << mount_point << "\n";
#else
            // Linux: mount the disc
            mount_point = "/tmp/burn_cme_mount_" + std::to_string(getpid());
            fs::create_directories(mount_point);
            
            std::cout << "\n[*] Mounting disc...\n";
            std::stringstream mount_cmd;
            mount_cmd << "mount -o ro " << shell_escape(current_drive) << " " << shell_escape(mount_point) << " 2>&1";
            result = execute_command(mount_cmd.str());
            
            if (result.find("mount:") != std::string::npos && result.find("permission") != std::string::npos) {
                std::cout << "[-] Mount requires root permissions.\n";
                std::cout << "[*] Trying with pmount...\n";
                mount_cmd.str("");
                mount_cmd << "pmount " << shell_escape(current_drive) << " burn_cme_disc 2>&1";
                result = execute_command(mount_cmd.str());
                mount_point = "/media/burn_cme_disc";
            }
            
            std::cout << result << "\n";
#endif
            
            std::cout << "[*] Copying files to " << output_directory << "...\n";
            
            std::stringstream copy_cmd;
            copy_cmd << "cp -rv " << shell_escape(mount_point + "/.") << " " << shell_escape(output_directory) << "/ 2>&1";
            result = execute_command(copy_cmd.str());
            std::cout << result << "\n";
            
#ifndef __APPLE__
            std::cout << "[*] Unmounting disc...\n";
            execute_command("umount " + shell_escape(mount_point) + " 2>/dev/null || pumount burn_cme_disc 2>/dev/null");
            fs::remove_all(mount_point);
#endif
            
            std::cout << "\n[+] Extraction complete.\n";
        }
        else if (option == "2") {
            std::cout << "\nEnter output ISO filename: ";
            std::string iso_name;
            std::getline(std::cin, iso_name);
            
            if (iso_name.empty()) {
                iso_name = "extracted_disc.iso";
            }
            
            if (iso_name.find(".iso") == std::string::npos) {
                iso_name += ".iso";
            }
            
            std::string output_path = output_directory + "/" + iso_name;
            
            std::cout << "\n[*] Creating ISO image from disc...\n";
            std::cout << "[*] This may take several minutes...\n\n";
            
            std::stringstream cmd;
            cmd << "dd if=" << shell_escape(current_drive) << " of=" << shell_escape(output_path) << " bs=2048 status=progress 2>&1";
            std::string result = execute_command(cmd.str());
            std::cout << result << "\n";
            
            if (fs::exists(output_path)) {
                auto size = fs::file_size(output_path);
                std::cout << "\n[+] ISO extracted: " << output_path;
                std::cout << " (" << (size / 1024.0 / 1024.0) << " MB)\n";
            } else {
                std::cout << "\n[-] Failed to extract ISO.\n";
            }
        }
    }
    
    void view_disc_info() {
        if (current_drive.empty()) {
            std::cout << "\n[*] Enter drive path (e.g., /dev/sr0): ";
            std::getline(std::cin, current_drive);
            if (current_drive.empty()) {
                current_drive = "/dev/sr0";
            }
        }
        
        std::cout << "\n[*] Disc Information for " << current_drive << "\n";
        std::cout << "─────────────────────────────────────────\n";
        
        std::cout << "\n[*] Drive capabilities:\n";
        std::stringstream cmd;
        cmd << "cdrecord dev=" << shell_escape(current_drive) << " -checkdrive 2>&1";
        std::string result = execute_command(cmd.str());
        std::cout << result << "\n";
        
        std::cout << "\n[*] Media info:\n";
        cmd.str("");
        cmd << "cdrecord dev=" << shell_escape(current_drive) << " -atip 2>&1";
        result = execute_command(cmd.str());
        std::cout << result << "\n";
        
        std::cout << "\n[*] Disc capacity:\n";
        cmd.str("");
        cmd << "dvd+rw-mediainfo " << shell_escape(current_drive) << " 2>&1 || echo 'dvd+rw-mediainfo not available'";
        result = execute_command(cmd.str());
        std::cout << result << "\n";
    }
    
    void erase_disc() {
        if (current_drive.empty()) {
            std::cout << "\n[-] No drive selected. Please detect drives first (option 1).\n";
            return;
        }
        
        std::cout << "\n[!] ERASE REWRITABLE DISC\n";
        std::cout << "─────────────────────────────────────────\n";
        std::cout << "[!] WARNING: This will permanently erase all data!\n";
        std::cout << "\nErase modes:\n";
        std::cout << "    1. Quick erase (fast)\n";
        std::cout << "    2. Full erase (slower, more thorough)\n";
        std::cout << "Select mode: ";
        
        std::string mode;
        std::getline(std::cin, mode);
        
        std::cout << "\n[!] Type 'ERASE' to confirm: ";
        std::string confirm;
        std::getline(std::cin, confirm);
        
        if (confirm != "ERASE") {
            std::cout << "[-] Erase cancelled.\n";
            return;
        }
        
        std::string blank_type = (mode == "2") ? "blank=all" : "blank=fast";
        
        std::cout << "\n[*] Erasing disc...\n";
        
        std::stringstream cmd;
        cmd << "cdrecord dev=" << shell_escape(current_drive) << " " << blank_type << " 2>&1";
        std::string result = execute_command(cmd.str());
        std::cout << result << "\n";
        
        std::cout << "\n[*] Erase process completed.\n";
    }
    
    void run() {
        display_header();
        
        std::string choice;
        while (true) {
            display_menu();
            std::getline(std::cin, choice);
            
            if (choice == "0") {
                std::cout << "\n[*] Thank you for using Burn-CME. Goodbye!\n\n";
                break;
            }
            else if (choice == "1") detect_drives();
            else if (choice == "2") add_files();
            else if (choice == "3") view_queue();
            else if (choice == "4") clear_queue();
            else if (choice == "5") create_iso();
            else if (choice == "6") burn_disc();
            else if (choice == "7") extract_data();
            else if (choice == "8") view_disc_info();
            else if (choice == "9") erase_disc();
            else if (choice == "A" || choice == "a") create_video_dvd();
            else if (choice == "B" || choice == "b") extract_dvd_video();
            else {
                std::cout << "\n[-] Invalid option. Please try again.\n";
            }
        }
    }
    
    void create_video_dvd() {
        std::cout << "\n[*] CREATE VIDEO DVD FROM MP4\n";
        std::cout << "─────────────────────────────────────────\n";
        
        if (!check_ffmpeg()) {
            std::cout << "[-] FFmpeg is not installed. Please install FFmpeg first.\n";
            std::cout << "    On Ubuntu/Debian: sudo apt install ffmpeg\n";
            std::cout << "    On macOS: brew install ffmpeg\n";
            return;
        }
        
        std::cout << "\nEnter path to MP4 file (or 'queue' to use video files in queue): ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) {
            std::cout << "[-] No input provided.\n";
            return;
        }
        
        std::cout << "Enter output directory for Video DVD [./video_dvd]: ";
        std::string output_dir;
        std::getline(std::cin, output_dir);
        if (output_dir.empty()) {
            output_dir = "./video_dvd";
        }
        
        if (!fs::exists(output_dir)) {
            fs::create_directories(output_dir);
        }
        
        std::cout << "\n[*] Converting video to DVD format...\n";
        std::cout << "[*] This may take a while depending on video length.\n\n";
        
        if (input == "queue") {
            std::vector<std::string> video_files;
            std::vector<std::string> video_extensions = {".mp4", ".avi", ".mkv", ".mov", ".wmv", ".flv", ".webm"};
            
            for (const auto& file : files_to_burn) {
                std::string ext = fs::path(file).extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                for (const auto& video_ext : video_extensions) {
                    if (ext == video_ext) {
                        video_files.push_back(file);
                        break;
                    }
                }
            }
            
            if (video_files.empty()) {
                std::cout << "[-] No video files found in queue.\n";
                return;
            }
            
            std::cout << "[*] Found " << video_files.size() << " video file(s) in queue.\n";
            convert_videos_to_dvd(video_files, output_dir);
        } else {
            if (!fs::exists(input)) {
                std::cout << "[-] File not found: " << input << "\n";
                return;
            }
            std::vector<std::string> files = {input};
            convert_videos_to_dvd(files, output_dir);
        }
    }
    
    bool check_ffmpeg() {
        std::string result = execute_command("which ffmpeg 2>/dev/null || command -v ffmpeg 2>/dev/null");
        return !result.empty() && result.find("not found") == std::string::npos;
    }
    
    void convert_videos_to_dvd(const std::vector<std::string>& video_files, const std::string& output_dir) {
        fs::path video_ts = fs::path(output_dir) / "VIDEO_TS";
        fs::create_directories(video_ts);
        
        std::vector<std::string> mpeg_files;
        int file_num = 1;
        
        for (const auto& input_file : video_files) {
            std::cout << "\n[*] Converting: " << fs::path(input_file).filename().string() << "\n";
            
            fs::path mpeg_file = fs::path(output_dir) / ("video_" + std::to_string(file_num++) + ".mpg");
            
            std::stringstream cmd;
            cmd << "ffmpeg -y -i " << shell_escape(input_file)
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
                std::cout << "[+] Created: " << mpeg_file.string() << "\n";
            } else {
                std::cout << "[-] Failed to convert: " << input_file << "\n";
                std::cout << result << "\n";
            }
        }
        
        if (mpeg_files.empty()) {
            std::cout << "\n[-] No files were converted successfully.\n";
            return;
        }
        
        std::string dvdauthor_check = execute_command("which dvdauthor 2>/dev/null");
        if (!dvdauthor_check.empty() && dvdauthor_check.find("not found") == std::string::npos) {
            std::cout << "\n[*] Creating DVD structure with dvdauthor...\n";
            
            fs::path dvd_xml = fs::path(output_dir) / "dvd.xml";
            std::ofstream xml_file(dvd_xml);
            xml_file << "<dvdauthor dest=\"" << output_dir << "\">\n";
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
            std::cout << result << "\n";
            
            fs::remove(dvd_xml);
        } else {
            std::cout << "\n[!] dvdauthor not installed. MPEG-2 files created but no DVD structure.\n";
            std::cout << "    Install dvdauthor: sudo apt install dvdauthor\n";
        }
        
        std::cout << "\n[+] Video DVD files created in: " << output_dir << "\n";
        std::cout << "\nWould you like to burn this Video DVD now? (y/n): ";
        std::string burn_choice;
        std::getline(std::cin, burn_choice);
        
        if (burn_choice == "y" || burn_choice == "Y") {
            std::cout << "\n[*] Creating ISO from Video DVD...\n";
            std::string iso_path = output_dir + "/video_dvd.iso";
            
            std::stringstream iso_cmd;
            iso_cmd << "mkisofs -dvd-video -V 'VIDEO_DVD' -o "
                    << shell_escape(iso_path) << " "
                    << shell_escape(output_dir) << " 2>&1";
            
            std::string result = execute_command(iso_cmd.str());
            std::cout << result << "\n";
            
            if (fs::exists(iso_path)) {
                std::cout << "[+] ISO created: " << iso_path << "\n";
                std::cout << "[*] Burning to disc...\n";
                
                std::string drive = current_drive.empty() ? "/dev/sr0" : current_drive;
                std::stringstream burn_cmd;
                burn_cmd << "cdrecord -v dev=" << shell_escape(drive) << " " << shell_escape(iso_path) << " 2>&1";
                result = execute_command(burn_cmd.str());
                std::cout << result << "\n";
            }
        }
    }
    
    void extract_dvd_video() {
        std::cout << "\n[*] EXTRACT DVD VIDEO TO MP4\n";
        std::cout << "─────────────────────────────────────────\n";
        
        if (!check_ffmpeg()) {
            std::cout << "[-] FFmpeg is not installed. Please install FFmpeg first.\n";
            return;
        }
        
        std::cout << "\nOptions:\n";
        std::cout << "  1. Extract from DVD drive\n";
        std::cout << "  2. Extract from VIDEO_TS folder\n";
        std::cout << "  3. Extract from ISO file\n";
        std::cout << "Select option: ";
        
        std::string option;
        std::getline(std::cin, option);
        
        std::string source_path;
        bool need_mount = false;
        
        if (option == "1") {
            source_path = current_drive.empty() ? "/dev/sr0" : current_drive;
            need_mount = true;
        } else if (option == "2") {
            std::cout << "Enter path to VIDEO_TS folder: ";
            std::getline(std::cin, source_path);
        } else if (option == "3") {
            std::cout << "Enter path to ISO file: ";
            std::getline(std::cin, source_path);
            need_mount = true;
        } else {
            std::cout << "[-] Invalid option.\n";
            return;
        }
        
        std::cout << "Enter output MP4 filename [extracted_video.mp4]: ";
        std::string output_path;
        std::getline(std::cin, output_path);
        if (output_path.empty()) {
            output_path = "extracted_video.mp4";
        }
        
        std::string mount_point;
        std::string vob_source;
        bool did_mount = false;
        
        if (need_mount) {
            std::cout << "\n[*] Accessing source...\n";
            
#ifdef __APPLE__
            if (option == "1") {
                // DVD drive on macOS - find auto-mounted volume
                std::string drive = source_path;
                if (drive.find("/dev/disk") != std::string::npos) {
                    std::string mount_info = execute_command("diskutil info " + shell_escape(drive) + " 2>/dev/null | grep 'Mount Point'");
                    size_t colon_pos = mount_info.find(':');
                    if (colon_pos != std::string::npos) {
                        mount_point = mount_info.substr(colon_pos + 1);
                        mount_point.erase(0, mount_point.find_first_not_of(" \t\n\r"));
                        mount_point.erase(mount_point.find_last_not_of(" \t\n\r") + 1);
                    }
                }
                // Search /Volumes for VIDEO_TS if not found
                if (mount_point.empty() || !fs::exists(mount_point)) {
                    for (const auto& entry : fs::directory_iterator("/Volumes")) {
                        std::string vol_path = entry.path().string();
                        if (fs::exists(vol_path + "/VIDEO_TS")) {
                            mount_point = vol_path;
                            break;
                        }
                    }
                }
                if (mount_point.empty()) {
                    std::cout << "[-] Could not find mounted DVD.\n";
                    return;
                }
            } else {
                // ISO file on macOS - use hdiutil
                std::string attach_result = execute_command("hdiutil attach " + shell_escape(source_path) + " -nobrowse 2>&1");
                // Parse mount point from output
                std::istringstream iss(attach_result);
                std::string line;
                while (std::getline(iss, line)) {
                    if (line.find("/Volumes/") != std::string::npos) {
                        size_t vol_pos = line.find("/Volumes/");
                        mount_point = line.substr(vol_pos);
                        mount_point.erase(mount_point.find_last_not_of(" \t\n\r") + 1);
                        break;
                    }
                }
                if (mount_point.empty()) {
                    std::cout << "[-] Failed to mount ISO.\n";
                    return;
                }
                did_mount = true;
            }
            vob_source = mount_point;
#else
            // Linux
            mount_point = "/tmp/burn_cme_dvd_mount";
            fs::create_directories(mount_point);
            
            std::stringstream mount_cmd;
            if (option == "1") {
                mount_cmd << "mount -t udf,iso9660 " << shell_escape(source_path) << " " << shell_escape(mount_point) << " 2>&1";
            } else {
                mount_cmd << "mount -o loop " << shell_escape(source_path) << " " << shell_escape(mount_point) << " 2>&1";
            }
            std::string result = execute_command(mount_cmd.str());
            did_mount = true;
            vob_source = mount_point;
#endif
        } else {
            vob_source = source_path;
        }
        
        fs::path video_ts = fs::path(vob_source);
        if (video_ts.filename() != "VIDEO_TS") {
            video_ts = video_ts / "VIDEO_TS";
        }
        
        if (!fs::exists(video_ts)) {
            std::cout << "[-] VIDEO_TS folder not found.\n";
            if (did_mount) {
#ifdef __APPLE__
                execute_command("hdiutil detach " + shell_escape(mount_point) + " 2>/dev/null");
#else
                execute_command("umount " + shell_escape(mount_point) + " 2>&1");
#endif
            }
            return;
        }
        
        std::vector<std::string> vob_files;
        for (const auto& entry : fs::directory_iterator(video_ts)) {
            std::string filename = entry.path().filename().string();
            if (filename.find("VTS_") == 0 && filename.find(".VOB") != std::string::npos) {
                vob_files.push_back(entry.path().string());
            }
        }
        
        std::sort(vob_files.begin(), vob_files.end());
        
        if (vob_files.empty()) {
            std::cout << "[-] No VOB files found.\n";
            if (did_mount) {
#ifdef __APPLE__
                execute_command("hdiutil detach " + shell_escape(mount_point) + " 2>/dev/null");
#else
                execute_command("umount " + shell_escape(mount_point) + " 2>&1");
#endif
            }
            return;
        }
        
        std::cout << "[*] Found " << vob_files.size() << " VOB files.\n";
        std::cout << "[*] Converting to MP4 (this may take a while)...\n\n";
        
        std::string concat_input = "concat:";
        for (size_t i = 0; i < vob_files.size(); i++) {
            if (i > 0) concat_input += "|";
            concat_input += vob_files[i];
        }
        
        std::stringstream cmd;
        cmd << "ffmpeg -y -i " << shell_escape(concat_input)
            << " -c:v libx264 -preset medium -crf 23"
            << " -c:a aac -b:a 192k"
            << " -movflags +faststart"
            << " " << shell_escape(output_path)
            << " 2>&1";
        
        std::string result = execute_command(cmd.str());
        
        if (did_mount) {
#ifdef __APPLE__
            execute_command("hdiutil detach " + shell_escape(mount_point) + " 2>/dev/null");
#else
            execute_command("umount " + shell_escape(mount_point) + " 2>&1");
#endif
        }
        
        if (fs::exists(output_path) && fs::file_size(output_path) > 0) {
            std::cout << "\n[+] Successfully extracted to: " << output_path << "\n";
            std::cout << "[+] File size: " << (fs::file_size(output_path) / (1024*1024)) << " MB\n";
        } else {
            std::cout << "\n[-] Extraction failed.\n";
            std::cout << result << "\n";
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--version" || arg == "-v") {
            std::cout << "Burn-CME version 1.0\n";
            std::cout << "CD/DVD Burning and Extraction Utility\n";
            return 0;
        }
        else if (arg == "--help" || arg == "-h") {
            std::cout << "Burn-CME - CD/DVD Burning and Extraction Utility\n\n";
            std::cout << "Usage: burn-cme [OPTIONS]\n\n";
            std::cout << "Options:\n";
            std::cout << "  -h, --help     Show this help message\n";
            std::cout << "  -v, --version  Show version information\n\n";
            std::cout << "Features:\n";
            std::cout << "  - Detect CD/DVD drives\n";
            std::cout << "  - Add files and create burn queue\n";
            std::cout << "  - Create ISO images from files\n";
            std::cout << "  - Burn ISO images to CD/DVD\n";
            std::cout << "  - Extract data from CD/DVD\n";
            std::cout << "  - View disc information\n";
            std::cout << "  - Erase rewritable discs\n";
            std::cout << "  - Create Video DVD from MP4 files\n";
            std::cout << "  - Extract DVD video content to MP4\n";
            return 0;
        }
    }
    
    BurnCME app;
    app.run();
    
    return 0;
}
