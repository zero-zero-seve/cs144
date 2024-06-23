#include "reassembler.hh"

using namespace std;

void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring) {
    // Your code here.
    (void)first_index;
    (void)data;
    (void)is_last_substring;
    //TODO: 判断是否缓存了最后一个字符串，如果缓存成功，之后只需要每次调用执行写函数
    // 对收到数据做判断，决定是否要缓存
    if (first_index < expectIndex) {          // 收到已经缓存下来数据
    } else if (first_index == expectIndex) {  // 收到期待数据包，判断是否用剩余空间存储它
        if (storeMap.size() < limit) {        // 允许存储
            storeMap[first_index] = data;
        }
    } else {  // 收到未来期望收到数据包，判断是否在窗口范围之内以及是否有足够容量，决定是否接收
        if (first_index < expectIndex + 4 * 1460 && storeMap.size() < limit) {
            storeMap[first_index] = data;
        }
    }
    // 能写则写
    // 创建一个写者，只要还能写就写
    uint64_t writeCount = 0;
    uint64_t temp;
    while (output_.writer().available_capacity() != 0 &&
           storeMap.find(output_.writer().bytes_pushed()) != storeMap.end()) {
        temp = output_.writer().bytes_pushed();
        writeCount = output_.writer().push(storeMap.at(temp));
        // 写完之后做更新，查看有没有写完这一整个string
        if (writeCount == storeMap.at(temp).size()) {
            storeMap.erase(temp);
        } else {
            storeMap.erase(temp);
            storeMap[output_.writer().bytes_pushed()] =
                storeMap[temp].substr(output_.writer().bytes_pushed());
            break;
        }
    }

    

}

uint64_t Reassembler::bytes_pending() const {
    // Your code here.
    return {};
}
