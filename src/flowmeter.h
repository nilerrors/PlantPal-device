struct FlowMeter {
  const uint8_t PIN;
  uint32_t pulses_count;
  float total_milli_litres;
  bool running;
  unsigned long last_running;
};
