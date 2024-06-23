#include "reassembler.hh"

using namespace std;

void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring) {
    // Your code here.
    (void)first_index;
    (void)data;
    (void)is_last_substring;
    //TODO: �ж��Ƿ񻺴������һ���ַ������������ɹ���֮��ֻ��Ҫÿ�ε���ִ��д����
    // ���յ��������жϣ������Ƿ�Ҫ����
    if (first_index < expectIndex) {          // �յ��Ѿ�������������
    } else if (first_index == expectIndex) {  // �յ��ڴ����ݰ����ж��Ƿ���ʣ��ռ�洢��
        if (storeMap.size() < limit) {        // ����洢
            storeMap[first_index] = data;
        }
    } else {  // �յ�δ�������յ����ݰ����ж��Ƿ��ڴ��ڷ�Χ֮���Լ��Ƿ����㹻�����������Ƿ����
        if (first_index < expectIndex + 4 * 1460 && storeMap.size() < limit) {
            storeMap[first_index] = data;
        }
    }
    // ��д��д
    // ����һ��д�ߣ�ֻҪ����д��д
    uint64_t writeCount = 0;
    uint64_t temp;
    while (output_.writer().available_capacity() != 0 &&
           storeMap.find(output_.writer().bytes_pushed()) != storeMap.end()) {
        temp = output_.writer().bytes_pushed();
        writeCount = output_.writer().push(storeMap.at(temp));
        // д��֮�������£��鿴��û��д����һ����string
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
