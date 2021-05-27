#include "choose.h"

int choose() {
  printf("--------------------------\n");
  printf("|-------1.register-------|\n");
  printf("|-------2.delete---------|\n");
  printf("|-------3.modify---------|\n");
  printf("Please input your chooice:\n");
  int chooice;
  scanf("%d", &chooice);
  return chooice;
}
