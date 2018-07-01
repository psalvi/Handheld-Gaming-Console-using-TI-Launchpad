unsigned short lfsr = 0xACE1u;
  unsigned short bit;

  unsigned short rand()
  {
    bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
    return lfsr =  (lfsr >> 1) | (bit << 15);
  }
	




	