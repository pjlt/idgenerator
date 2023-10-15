/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2023 Zhennan Tu <zhennan.tu@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <random>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <filesystem>


bool validNum(uint32_t num) {
    static const std::vector<uint32_t> kInvalidNums{ 111, 222, 333, 444, 555, 666, 777, 888 };
    for (auto& n : kInvalidNums) {
        if (n == num) {
            return false;
        }
    }
    return true;
}

bool gen() {
    // 1. Create three parts
    std::vector<uint32_t> first;
    for (uint32_t i = 100; i < 999; i++) {
        if (validNum(i)) {
            first.push_back(i);
        }
    }
    std::vector<uint32_t> second;
    for (uint32_t i = 1; i < 999; i++) {
        if (validNum(i)) {
            second.push_back(i);
        }
    }
    std::vector<uint32_t>& third = second;

    //2. Allocate memory
    std::vector<uint32_t> total;
    size_t total_size = first.size() * second.size() * third.size();
    std::cout << "Start memory allocation for " << total_size << " ids" << std::endl;
    auto start = std::chrono::steady_clock::now();
    total.resize(total_size); // No more than 900 * 1000 * 1000 * sizeof(u32) => 3433 MBytes
    auto end = std::chrono::steady_clock::now();
    auto used_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Memory allocation finished, used " << used_time_ms << "ms" << std::endl;

    //3. Fiil numbers
    std::cout << "Start fill numbers..." << std::endl;
    start = std::chrono::steady_clock::now();
    size_t index = 0; //cache friendly
    for (auto f : first) {
        for (auto s : second) {
            for (auto t : third) {
                total[index++] = f * 1'000'000 + s * 1'000 + t;
            }
        }
    }
    end = std::chrono::steady_clock::now();
    used_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Fill numbers finished, used " << used_time_ms << "ms" << std::endl;

    //4. Shuffle
    std::random_device rd;
    std::mt19937 rand_engine(rd());
    std::cout << "Start shuffle..." << std::endl;
    start = std::chrono::steady_clock::now();
    std::shuffle(total.begin(), total.end(), rand_engine);
    end = std::chrono::steady_clock::now();
    used_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Shuffle finished, used " << used_time_ms << "ms" << std::endl;

    //5. Write files
    std::string alphabet = "abcdefghijklmnopqrstuvwxzy";
    std::filesystem::path root{ "./ids" };
    if (!std::filesystem::create_directory(root)) {
        std::cout << "Create directory " << root.c_str() << " failed" << std::endl;
        return false;
    }
    index = 0;
    for (size_t i = 0; i < 26; i++) {
        std::filesystem::path dir1 = root / alphabet.substr(i, 1);
        if (!std::filesystem::create_directory(dir1)) {
            std::cout << "Create directory " << dir1.c_str() << " failed" << std::endl;
            return false;
        }
        for (size_t j = 0; j < 26; j++) {
            std::filesystem::path dir2 = dir1 / alphabet.substr(j, 1);
            if (!std::filesystem::create_directory(dir2)) {
                std::cout << "Create directory " << dir2.c_str() << " failed" << std::endl;
                return false;
            }
            for (size_t k = 0; k < 10; k++) {
                size_t end_index = index + 200'000; // not included
                end_index = std::min(total.size(), end_index);
                size_t count = end_index - index;
                if (count == 0) {
                    return true;
                }
                std::string filename = "id_xxx";
                filename[3] = alphabet[i];
                filename[4] = alphabet[j];
                filename[5] = alphabet[k];
                std::filesystem::path filepath = dir2 / filename;
                FILE* file = fopen(filepath.string().c_str(), "wb+");
                if (file == nullptr) {
                    std::cout << "fopen " << filepath.string() << " failed" << std::endl;
                    return false;
                }
                fwrite(&total[index], sizeof(uint32_t), count, file);
                fclose(file);
                index = end_index;
            }
        }
    }
}


int main()
{
    auto start_program_time = std::chrono::steady_clock::now();
    if (!gen()) {
        return -1;
    }
    auto end_program_time = std::chrono::steady_clock::now();
    auto total_use_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_program_time - start_program_time).count();
    std::cout << "All done, total used " << total_use_ms << "ms" << std::endl;

    return 0;
}