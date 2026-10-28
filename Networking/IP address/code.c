#include <stdio.h>
#include <stdlib.h>

int main() {
  char ip[100];
  
  printf("\nEnter a IP address: ");
  scanf("%s",ip);
  
  char class;
  
  int i = 0;
  int x = 0;
  int nID, hID;
  
  char dba[20] = "";
  
  while(ip[i] != '.') {
    x = x * 10 + (ip[i++] - '0');
  }
  
  if(x >= 0 && x <= 123) {
    class = 'A';
    nID = 8;
    hID = 24;
    
    int count = 0, i = 0, j = 0;
    while(count != 1) {
      if(ip[i] == '.') {
        count++;
        dba[j++] = ip[i++];
      }
      else dba[j++] = ip[i++];
    }
    
    for(int k=0; k<3; k++) {
      dba[j++] = '2';
      dba[j++] = '5';
      dba[j++] = '5';
      dba[j++] = '.';
    }
    
    dba[j--] = '\n';
  }
  else if(x >= 124 && x <= 193) {
    class = 'B';
    nID = 16;
    hID = 16;
    
    int count = 0, i = 0, j = 0;
    while(count != 2) {
      if(ip[i] == '.') {
        count++;
        dba[j++] = ip[i++];
      }
      else dba[j++] = ip[i++];
    }
    
    for(int k=0; k<2; k++) {
      dba[j++] = '2';
      dba[j++] = '5';
      dba[j++] = '5';
      dba[j++] = '.';
    }
    
    dba[j--] = '\n';
  }
  else if(x >= 194 && x <= 223) {
    class = 'C';
    nID = 24;
    hID = 8;
    
    int count = 0, i = 0, j = 0;
    while(count != 3) {
      if(ip[i] == '.') {
        count++;
        dba[j++] = ip[i++];
      }
      else dba[j++] = ip[i++];
    }
    
    for(int k=0; k<1; k++) {
      dba[j++] = '2';
      dba[j++] = '5';
      dba[j++] = '5';
      dba[j++] = '.';
    }
    
    dba[j--] = '\n';
  }
  else if(x >= 224 && x <= 239) {
    class = 'D';
    nID = -1;
    hID = -1;
  }
  else if(x >= 240 && x <= 255) {
    class = 'E';
    nID = -1;
    hID = -1;
  }
  else class = 'X';
  
  if(class == 'X') {
    printf("\nWrong IP address\n\n");
  }
  
  printf("\nClass: %c", class);
  printf("\nNID: %d", nID);
  printf("\nHID: %d", hID);
  printf("\nNo of N/w: 2^%d", nID-1);
  printf("\nNo of Host N/w: 2^%d - 2", hID);
  printf("\nDBA: %s\n", dba);

  return 0;
}
