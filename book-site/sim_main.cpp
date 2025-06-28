#include <iostream>
#include <cstdint>
#include <Vtop.h>
#include <random>
#include <bitset>

#define RESET 100
#define SET_NUM 100
#define LOGIC_R_SHIFT 100
#define LOGIC_L_SHIFT 100
#define ARITH_R_SHIFT 5
#define ONEIN_8OUT 10
#define ROTATE_L_SHIFT 10


unsigned char rotate_l_shift(int nbit, unsigned char data) {
 
  for(int i = 0 ; i < nbit; i++) {

    unsigned char msb = (data >> 7);
    data = data << 1;
    data = data | msb; 
  }
  return data;
}

int main() {

  Vtop *top = new Vtop;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned char> rdata(0, 255);
  std::uniform_int_distribution<int> rshamt(0,7);
  
  
  top->clock = 0;
  /* 000: reset */
  int RESET_fail = 0;
  for(int i = 0; i <= RESET ;i++) {
    
    top->clock = !top->clock;
    top->io_op = 0;

    top->eval();

    if(top->io_par_out != 0) {
      RESET_fail ++;
    }
  }

  /* 001: set number */
  unsigned char last_data = 0;
  top->clock = 0;
   top->eval();
  int SET_NUM_fail = 0;
  for(int i = 0; i <= RESET; i++) {
    
    unsigned char data = rdata(gen);
    top->io_par_in = data;
    top->io_op = 1;
    top->clock = !top->clock;
    top->eval();

    /* if i is even number, then a positive edge occurs. */
    if(!(i % 2)){
      if(data != top->io_par_out) {
         SET_NUM_fail ++;
         std::cout << "------------Error-------------"<< std::endl;
         std::cout << "i = "          << i
                  << ",data = "       << std::bitset<8>(data) 
                  << ",io_par_out = " << std::bitset<8>(top->io_par_out)
                  << std::endl;
      }
      last_data = data;
    }
    /* i is a odd number. */
    else {
       if(top->io_par_out != last_data) {
         SET_NUM_fail ++;
         std::cout << "------------Error-------------" << std::endl;
         std::cout << "i = "          << i
                  << "last_data = "  << std::bitset<8>(last_data) 
                  << ",io_par_out = " << std::bitset<8>(top->io_par_out)
                  << std::endl;

       }
    }

  }
  
  /* 010: Logic right shift */
   int LOGIC_R_SHIFT_fail  = 0;
  for (int i = 0; i < LOGIC_R_SHIFT; i++) {

    /* Write into a byte*/
    top->clock = 0;
    top->eval();
    unsigned char data = rdata(gen);
    top->io_op = 1;
    top->io_par_in = data;
    top->clock = 1;
    top->eval();

    std::cout << "-------------------right shift:" << std::bitset<8> (data) << "---------------------" << std::endl;
    top->clock = 0;
    top->eval();
    /* Shift this byte. */
    /* j=14 means the appearance of the 8th postive edge */
    for (int j = 0; j <= 14; j ++)
    { 
      top->io_op = 2;
      top->clock = !top->clock;
      top->eval();
      if(top->io_par_out != data >> (j / 2)+1)
      {
        std::cout << "top->io_par_out != data >> j / 2" << std::endl;
        LOGIC_R_SHIFT_fail ++;
      }
      /* Print shifted data when a postive edge occurs. */
      if(!(j % 2))
      {
        std::cout << "right shift " << (j / 2) +1 << "bit:" << std::bitset<8> (top->io_par_out) << std::endl;
      }
    }

  }
  
  /* 011: Logic left shitf */
  int LOGIC_L_SHIFT_fail  = 0;
  for (int i = 0; i < LOGIC_L_SHIFT; i++) {

    /* Write into a byte*/
    top->clock = 0;
    top->eval();
    unsigned char data = rdata(gen);
    top->io_op = 1;
    top->io_par_in = data;
    top->clock = 1;
    top->eval();

    std::cout << "-------------------logic left shift:" << std::bitset<8> (data) << "---------------------" << std::endl;
    top->clock = 0;
    top->eval();
    /* Shift this byte. */
    /* j=14 means the appearance of the 8th postive edge */
    for (int j = 0; j <= 14; j ++)
    { 
      top->io_op = 3;
      top->clock = !top->clock;
      top->eval();
      std::cout << "top->io_par_out type :" << typeid(top->io_par_out).name() 
	       << "(data << (j / 2)+1)) type" << typeid((data << (j / 2)+1)).name()  << std::endl;
      if(static_cast<unsigned char> (top->io_par_out) !=static_cast<unsigned char> (data << (j / 2)+1))
      {
        std::cout << "top->io_par_out:" <<  std::bitset<8> (top->io_par_out) << "!= data <<" << (j/2)+1 << ": "  << std::bitset<8>(data<<(j/2)+1)  << std::endl;
        LOGIC_L_SHIFT_fail ++;
      }
      /* Print shifted data when a postive edge occurs. */
      if(!(j % 2))
      {
        std::cout << "logic left shift " << (j / 2) +1 << "bit:" << std::bitset<8> (top->io_par_out) << std::endl;
      }
    }


  /* Arithmetic right shift */
  int ARITH_R_SHIFT_fail  = 0;
  for (int i = 0; i < ARITH_R_SHIFT; i++) {

    /* Write into a byte*/
    top->clock = 0;
    top->eval();
    signed char data = rdata(gen);
    top->io_op = 1;
    top->io_par_in = data;
    top->clock = 1;
    top->eval();

    std::cout << "-------------------arithmetic right shift:" << std::bitset<8> (data) << "---------------------" << std::endl;
    top->clock = 0;
    top->eval();
    /* Shift this byte. */
    /* j=14 means the appearance of the 8th postive edge */
    for (int j = 0; j <= 14; j ++)
    { 
      top->io_op = 4;
      top->clock = !top->clock;
      top->eval();
      if(top->io_par_out != static_cast<unsigned char> (data >> (j / 2)+1))
      {
        std::cout << "top->io_par_out != data >> j / 2" << std::endl;
        ARITH_R_SHIFT_fail++;
      }
      /* Print shifted data when a postive edge occurs. */
      if(!(j % 2))
      {
        std::cout << "arithmetic right shift " << (j / 2) +1 << "bit:" << std::bitset<8> (top->io_par_out) << std::endl;
      }
    }

  }
 
  std::uniform_int_distribution<unsigned char> rbit(0, 1);
  /* 101: 1 bit in, 8 bit out and right shift */
  int ONEIN_8OUT_fail = 0;
  for(int i = 0; i < ONEIN_8OUT; i++) {
    /* Reset shift register */
    top->clock = 0;
    top->eval();
    top->io_op = 0;
    top->clock = 1;    
    top->eval();
 
    top->clock = 0;
    top->eval();
    unsigned char d = 0;
    std::cout << "-------------------1bit in, 8bit out---------------------" << std::endl;
    for(int j = 0; j<=14; j++) {
      unsigned char r_bit = rbit(gen);
      top->io_din = r_bit; 
      top->clock  = !top->clock;
      top->io_op = 5;
      top->eval();


      /* When positive edge occurs... */
      if(!(j % 2)) {
        d = d >> 1;
        d = d | (r_bit << 7); 
        std::cout << (j/2) + 1 			<< "th cycle:"
                 << "  Input bit:"		<< std::bitset<1>(r_bit)
                 << "  Rigister output: "	<< std::bitset<8>(top->io_par_out)
    		 << "  Your result:"		<< std::bitset<8>(d)
		 << std::endl;

       if(top->io_par_out != static_cast<unsigned char> (d))
       { 
          std::cout << "1bit in, 8bit out Error."<< std::endl;
	  ONEIN_8OUT_fail ++;
       }
      }
    }

  
  }
  
  /* 110: ROTATE LEFT SHIFT */
  int ROTATE_L_SHIFT_fail = 0;
  for (int i = 0; i <= ROTATE_L_SHIFT; i++) {
   
    /* Write into a byte*/
    top->clock = 0;
    top->eval();
    signed char data = rdata(gen);
    top->io_op = 1;
    top->io_par_in = data;
    top->clock = 1;
    top->eval();

    std::cout << "-----------------  rotate left shift:" << std::bitset<8> (data) << "---------------------" << std::endl;
    top->clock = 0;
    top->eval();
    /* Shift this byte. */
    for (int j = 0; j <= 20; j ++)
    { 
      top->io_op = 7;
      top->clock = !top->clock;
      top->eval();
     if(top->io_par_out != static_cast<unsigned char> ( rotate_l_shift ( (j/2) + 1 , data) ))
      {
        std::cout << "top->io_par_out != data >> j / 2" << std::endl;
        ROTATE_L_SHIFT_fail ++;
      }
      /* Print shifted data when a postive edge occurs. */
      if(!(j % 2))
      {
        std::cout << "rotate right shift " << (j / 2) +1 << "bit:" << std::bitset<8> (top->io_par_out) << std::endl;
      }
    }

  }
  

  std::cout << "RESET_fail:" << RESET_fail << std::endl; 
  std::cout << "SET_NUM_fail:" << SET_NUM_fail << std::endl; 
  std::cout << "LOGIC_R_SHIFT_fail:" << LOGIC_R_SHIFT_fail << std::endl; 
  std::cout << "LOGIC_L_SHIFT_fail:" << LOGIC_L_SHIFT_fail << std::endl; 
  std::cout << "ARITH_R_SHIFT_fail" <<  ARITH_R_SHIFT_fail << std::endl; 
  std::cout << "ONEIN_8OUT_fail:" <<  ONEIN_8OUT_fail << std::endl; 
  std::cout << "ROTATE_L_SHIFT_fail:" <<  ROTATE_L_SHIFT_fail << std::endl; 
}}
