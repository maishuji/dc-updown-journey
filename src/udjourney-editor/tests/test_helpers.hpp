// Copyright 2025 Quentin Cartier
#pragma once
#include <filesystem>
#include <fstream>
#include <string>

// Helper function to create a temporary test file
inline std::string create_temp_file(const std::string& content) {
    std::string temp_path = std::filesystem::temp_directory_path() /
                            ("test_" + std::to_string(rand()) + ".json");
    std::ofstream file(temp_path);
    file << content;
    file.close();
    return temp_path;
}

// Helper function to read file content
inline std::string read_file(const std::string& path) {
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
}
