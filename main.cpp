#include <iostream>
#include <string_view>
#include <string.h>
#include <forward_list>
#include <string>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <bit>
#include <codecvt>
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <vector>
#include <list>
#include <mach/mach.h>


size_t getMemoryUsage() {
    task_basic_info_data_t taskInfo;
    mach_msg_type_number_t infoCount = TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&taskInfo, &infoCount) == KERN_SUCCESS) {
        return taskInfo.resident_size; // RAM usage in bytes
    }
    return 0;
}

void trainer(){
    int iterations = 32000;
    std::ifstream corpusText("input.txt");
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::unordered_map<std::u32string, std::forward_list<std::pair<int, std::list<std::u32string>::iterator>>> pairs;
    //std::unordered_map<std::u32string, std::u32string> pairComponents;
    std::unordered_map<std::u32string, int> pairFrequencies;
    std::unordered_map<std::u32string, int*> newFrequencies;
    std::priority_queue<std::pair<int, std::u32string>> pq;
    std::vector<std::list<std::u32string>> lines;
    std::list<std::u32string> line_list;
    std::u32string u32line;
    std::string line;
    char32_t replacement;
    
    while(getline(corpusText, line)){
        u32line = converter.from_bytes(line);
        for(int i = 0; i < u32line.length(); i++){
            line_list.push_back(u32line.substr(i, 1));
        }
        lines.push_back(line_list);
        line_list.clear();
        u32line.clear();
    }
    for(int i = 0; i < lines.size(); i++){
        auto end = --lines[i].end();
        for(auto it = lines[i].begin(); it != end; ++it){
            auto first = it;
            auto second = ++it;
            --it;
            pairs[*first + *second].push_front({i, first});
            pairFrequencies[*first + *second]++;
        }
    }
    
    for(auto pair : pairFrequencies){
        pq.push({pair.second, pair.first});
    }
    
    
    for(int i = 0; i < iterations; i++){
        replacement = static_cast<char32_t>(1000 + i);
        for(auto it = pairs[pq.top().second].begin(); it != pairs[pq.top().second].end(); ++it){
            if(*it->second != converter.from_bytes("")){
                if(*it->second == pq.top().second[0] + converter.from_bytes("")){
                    if(*it->second + *++it->second == pq.top().second){
                        --it->second;
                        if(it->second != lines[it->first].begin()){
                            auto prev = --(it->second);
                            auto current = ++(it->second);
                            auto next = ++(it->second);
                            int ten = 10;
                            pairFrequencies[*prev + *current]--;
                            pairFrequencies[*prev + replacement]++;
                            
                            if(newFrequencies[*prev + *current] != &pairFrequencies[*prev + *current]){
                                newFrequencies[*prev + *current] = &pairFrequencies[*prev + *current];
                            }
                            if(newFrequencies[*prev + replacement] != &pairFrequencies[*prev + replacement]){
                                newFrequencies[*prev + replacement] = &pairFrequencies[*prev + replacement];
                            }
                            if(it->second != --lines[it->first].end()){
                                auto nextnext = ++(it->second);
                                pairFrequencies[*next + *nextnext]--;
                                pairFrequencies[replacement + *nextnext]++;
                                
                                if(newFrequencies[*next + *nextnext] != &pairFrequencies[*next + *nextnext]){
                                    newFrequencies[*next + *nextnext] = &pairFrequencies[*next + *nextnext];
                                }
                                if(newFrequencies[replacement+ *nextnext] != &pairFrequencies[replacement+ *nextnext]){
                                    newFrequencies[replacement+ *nextnext] = &pairFrequencies[replacement+ *nextnext];
                                }
                                --it->second;
                                --it->second;
                                *it->second = replacement;
                                lines[it->first].erase(next);
                                pairs[*prev + *current].push_front({it->first, prev});
                                pairs[*current + *nextnext].push_front({it->first, current});
                                
                            }
                            else{
                                --it->second;
                                *it->second = replacement;
                                lines[it->first].erase(next);
                                pairs[*prev + *current].push_front({it->first, prev});
                            }
                        }
                        else{
                            auto current = (it->second);
                            auto next = ++(it->second);
                            if(it->second != lines[it->first].end()){
                                auto nextnext = ++(it->second);
                                pairFrequencies[*next + *nextnext]--;
                                pairFrequencies[replacement + *nextnext]++;
                                
                                if(newFrequencies[*next + *nextnext] != &pairFrequencies[*next + *nextnext]){
                                    newFrequencies[*next + *nextnext] = &pairFrequencies[*next + *nextnext];
                                }
                                if(newFrequencies[replacement+ *nextnext] != &pairFrequencies[replacement+ *nextnext]){
                                    newFrequencies[replacement+ *nextnext] = &pairFrequencies[replacement+ *nextnext];
                                }
                                
                                --it->second;
                                --it->second;
                                *it->second = replacement;
                                lines[it->first].erase(next);
                                pairs[*current + *nextnext].push_front({it->first, current});
                            }
                            else{
                                --it->second;
                            }
                        }
                    }
                    else{
                        --it->second;
                    }
                }
            }
        }
        pairs.erase(pq.top().second);
        pairFrequencies.erase(pq.top().second);
        //pairComponents.erase(pq.top().second);
        pq.pop();
        for(auto pair : newFrequencies){
            if(pairFrequencies[pair.first] != 0){
                pq.push({*pair.second, pair.first});
            }
        }
        newFrequencies.clear();
        while(pq.top().first != pairFrequencies[pq.top().second]){
            pq.pop();
        }
    }
}


int main() {
    auto start = std::chrono::high_resolution_clock::now();
    trainer();
    size_t ramUsage = getMemoryUsage();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "RAM Usage: " << ramUsage << " bytes" << std::endl;
    std::cout << "Time: " << std::chrono::duration<double>(end - start).count() << " s" << std::endl;
    return 0;
}
