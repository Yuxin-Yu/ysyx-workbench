#include <nvboard.h>
#include "Vtop.h"

static TOP_NAME dut;

vluint64_t main_time = 0;

void nvboard_bind_all_pins(Vtop* top);

double sc_time_stamp()
{
    return main_time;
}

int main() {

  dut.a = 0;
  dut.b = 0;
  dut.rst_n = 0;
  dut.clk = 0;

  nvboard_bind_all_pins(&dut);
  nvboard_init();

  while(1){

    //contextp->timeInc(1);  // 1 timeprecision period passes...

    // Toggle a fast (time/2 period) clock
    dut.clk = !dut.clk;

    if (main_time > 4) {
        dut.rst_n = 1;
    }
    if(dut.clk == 0){
      dut.a = rand()&1;
      dut.b = rand()&1;
    }
    
    //dut.b = 1;
    dut.eval();

    //printf("a = %d,b = %d,f = %d \n  ",dut.a,dut.b,dut.f);

    main_time++;
    nvboard_update();
  }
  
  //dut.final();
  
  return 0;
}
