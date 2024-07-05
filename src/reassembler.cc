#include "reassembler.hh"

using namespace std;

void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring) {
    this->doc++;
    // �õ��ֽ�Ƭ��֮�󣬶��ֽ�Ƭ������ȡ��������ֱ�Ӷ���
    uint64_t first_unassemabled_index = output_.writer().bytes_pushed();
    uint64_t first_unacceptable_index = first_unassemabled_index + output_.writer().available_capacity();
    uint64_t new_first_index = first_index;
    uint64_t new_last_index = first_index + data.size();
    // ȷ���ڷ�Χ�����ֽ�λ��
    if (first_index < first_unassemabled_index) {
        if (first_index + data.length() <= first_unassemabled_index) {
            // ֮ǰ�Ѿ���õĳ���Ƭ��
            return;
        } else {
            new_first_index = first_unassemabled_index;
        }
    } else if (first_index >= first_unacceptable_index) {
        return;
    }
    // ȷ���ڷ�Χ�����һ���ֽ�λ��
    if (first_index + data.size() < first_unacceptable_index) {
        new_last_index = first_index + data.size();
    } else {
        new_last_index = first_unacceptable_index;
    }
    if (new_last_index < new_first_index) {
        return;
    }

    // ���󿴣����ü�
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
        // �����ǿյĻ��ߵ�һ�����ξ��������Ҳֱ࣬��pass,���Ҳ�
    } else {
        // ����һ���������������λIndex�������Ҳ�Ķ�
        if (Rightindex <= new_first_index) {
            // ��ȫ���غ�,pass�����Ҳ�
        } else if (Rightindex < new_last_index) {
            // �����غ���Ҫ�ü������Ҳ�
            new_first_index = Rightindex;
        } else {
            // ��ȫ���ǣ�ֱ�Ӷ���
            return;
        }
    }
    // ����һЩ����Ƭ��
    for (; it != byteQueue.end();) {
        if (it->first < new_last_index) {
            // ����ĳЩƬ�������µ���Ƭ��֮��
            if ((it->first + it->second.length()) <= new_last_index) {
                // ��Ƭ�ο���ֱ�Ӷ���
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
        // �����ݴ洢������ǡ��λ��
        if (Leftindex == (uint64_t)-1) {
            // ����Ϊ�ջ��������Ԫ��
            byteQueue.push_front(std::pair<uint64_t, std::string>(new_first_index, data));
        } else if (old_it != byteQueue.end()) {
            byteQueue.insert(old_it, std::pair<uint64_t, std::string>(new_first_index, data));
        } else {
            byteQueue.push_back(std::make_pair(new_first_index, data));
        }
        // ����������������д��buffer
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
