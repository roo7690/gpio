#include "abstract.h"
#include "stdlib.h"
#include "stdio.h"

int _pin[]={-1,-1,8,-1,9,-1,7,15,-1,16,0,1,2,-1,3,4,-1,5,12,-1,13,6,14,10,-1,11,30,31,21,-1,22,26,23,-1,24,27,25,28,-1,29};

//gpio via pin physical board
A_gpio a_gpio(int pin){
  pin=_pin[pin-1];
  if(pin==-1){
    printf("\033[1;31mInvalid pin. Power ou Ground selected\033[0m\n");
    exit(1);
  }
  return pin;
}