#include <stdio.h>

int main() {
  char recv_data[10] = {0x00, 0x00, 0x00, 0x0B, 0x03,
                        0x0A, 0x00, 0x02, 0x0D, 0x0A};
  unsigned int id = 0;
  for (int i = 0; i < 10; ++i) {
    if (i >= 0 && i < 10) {
      // RFID卡的信息保存在第二位到第十一位
      /*
      recv_data[i] = (recv_data[i] > 0x40) ? (recv_data[i] - 0x41 + 10)
                                           : (recv_data[i] - 0x30);
      */
      // 数据拼接
      id = id << 4 | recv_data[i];
    }
  }
  printf("id = %x\n", id);
}
