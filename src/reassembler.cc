#include "reassembler.hh"

using namespace std;

void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring) {
    this->doc++;
    // 得到字节片段之后，对字节片段做截取操作或者直接丢掉
    uint64_t first_unassemabled_index = output_.writer().bytes_pushed();
    uint64_t first_unacceptable_index = first_unassemabled_index + output_.writer().available_capacity();
    uint64_t new_first_index = first_index;
    uint64_t new_last_index = first_index + data.size();
    // 确定在范围内首字节位置
    if (first_index < first_unassemabled_index) {
        if (first_index + data.length() <= first_unassemabled_index) {
            // 之前已经获得的程序片段
            return;
        } else {
            new_first_index = first_unassemabled_index;
        }
    } else if (first_index >= first_unacceptable_index) {
        return;
    }
    // 确定在范围内最后一个字节位置
    if (first_index + data.size() < first_unacceptable_index) {
        new_last_index = first_index + data.size();
    } else {
        new_last_index = first_unacceptable_index;
    }
    if (new_last_index < new_first_index) {
        return;
    }

    // 往左看，做裁剪
    uint64_t Leftindex = -1;
    uint64_t Rightindex = -1;
    auto it = byteQueue.begin();
    for (; it != byteQueue.end(); it++) {
        if (it->first <= new_first_index) {
            Leftindex = it->first;
            Rightindex = Leftindex + it->second.length();
        } else {
            break;
        }
    }
    auto old_it = it;
    if (Leftindex == (uint64_t)-1) {
        // 队列是空的或者第一个流段就在我们右侧，直接pass,看右侧
    } else {
        // 还有一种情况，不存在首位Index在我们右侧的段
        if (Rightindex <= new_first_index) {
            // 完全不重合,pass，看右侧
        } else if (Rightindex < new_last_index) {
            // 部分重合需要裁剪，看右侧
            new_first_index = Rightindex;
        } else {
            // 完全覆盖，直接丢掉
            return;
        }
    }
    // 处理一些覆盖片段
    for (; it != byteQueue.end();) {
        if (it->first < new_last_index) {
            // 存在某些片段在最新到来片段之间
            if ((it->first + it->second.length()) <= new_last_index) {
                // 该片段可以直接丢掉
                it = byteQueue.erase(it);
            } else {
                new_last_index = it->first;
                break;
            }
        } else {
            break;
        }
    }
    old_it = it;
    if (new_last_index < new_first_index) {
        return;
    }
    if (is_last_substring && new_last_index == (first_index + data.length())) {
        isGetLast = true;
    }
    if (new_first_index != new_last_index) {
        data = data.substr(new_first_index - first_index, new_last_index - new_first_index);
        // 将数据存储到队列恰当位置
        if (Leftindex == (uint64_t)-1) {
            // 队列为空或者左侧五元素
            byteQueue.push_front(std::pair<uint64_t, std::string>(new_first_index, data));
        } else if (old_it != byteQueue.end()) {
            byteQueue.insert(old_it, std::pair<uint64_t, std::string>(new_first_index, data));
        } else {
            byteQueue.push_back(std::make_pair(new_first_index, data));
        }
        // 将队列中数据整合写入buffer
        for (it = byteQueue.begin(); it != byteQueue.end();) {
            if (it->first == output_.writer().bytes_pushed()) {
                output_.writer().push(it->second);
                it = byteQueue.erase(it);
            } else {
                break;
            }
        }
    }
    if (isGetLast && byteQueue.empty()) {
        output_.writer().close();
    }
}
uint64_t Reassembler::bytes_pending() const {
    uint64_t sum = 0;
    for (auto it = byteQueue.begin(); it != byteQueue.end(); it++) {
        sum += it->second.length();
    }
    return sum;
}
