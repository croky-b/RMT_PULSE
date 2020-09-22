

#include "driver/rmt.h"

static unsigned long lastStateTime;
//static const char* TAG = "RMT";

#define RMT_TICK_PER_US 1
// determines how many clock cycles one "tick" is
// [1..255], source is generally 80MHz APB clk
#define RMT_RX_CLK_DIV (80000000/RMT_TICK_PER_US/1000000)
// time before receiver goes idle
#define RMT_RX_MAX_US 3500

#define RECEIVER_CHANNELS_NUM 6

extern volatile uint16_t ReceiverChannels[RECEIVER_CHANNELS_NUM];

volatile uint16_t ReceiverChannels[RECEIVER_CHANNELS_NUM] = {0};
const uint8_t RECEIVER_CHANNELS[RECEIVER_CHANNELS_NUM] = { 1, 2, 3, 4, 5, 6 };
const uint8_t RECEIVER_GPIOS[RECEIVER_CHANNELS_NUM] = { 13, 12, 14 ,27,33,35 };


// Reference https://esp-idf.readthedocs.io/en/v1.0/api/rmt.html

static void rmt_isr_handler(void* arg){
    
    uint32_t intr_st = RMT.int_st.val;

    uint8_t i;
    for(i = 0; i < RECEIVER_CHANNELS_NUM; i++) {
        uint8_t channel = RECEIVER_CHANNELS[i];
        uint32_t channel_mask = BIT(channel*3+1);

        if (!(intr_st & channel_mask)) continue;

        RMT.conf_ch[channel].conf1.rx_en = 0;
        RMT.conf_ch[channel].conf1.mem_owner = RMT_MEM_OWNER_TX;
        volatile rmt_item32_t* item = RMTMEM.chan[channel].data32;
        if (item) {
            ReceiverChannels[i] = item->duration0;
        }

        RMT.conf_ch[channel].conf1.mem_wr_rst = 1;
        RMT.conf_ch[channel].conf1.mem_owner = RMT_MEM_OWNER_RX;
        RMT.conf_ch[channel].conf1.rx_en = 1;

        //clear RMT interrupt status.
        RMT.int_clr.val = channel_mask;
    }
}

 
void setup() 
{
   
    uint8_t i;
    rmt_config_t rmt_channels[RECEIVER_CHANNELS_NUM] = {};

    for (i = 0; i < RECEIVER_CHANNELS_NUM; i++) {
        rmt_channels[i].channel = (rmt_channel_t) RECEIVER_CHANNELS[i];
        rmt_channels[i].gpio_num = (gpio_num_t) RECEIVER_GPIOS[i];
        rmt_channels[i].clk_div = RMT_RX_CLK_DIV;
        rmt_channels[i].mem_block_num = 1;
        rmt_channels[i].rmt_mode = RMT_MODE_RX;
        rmt_channels[i].rx_config.filter_en = true;
        rmt_channels[i].rx_config.filter_ticks_thresh = 100;
        rmt_channels[i].rx_config.idle_threshold = RMT_RX_MAX_US * RMT_TICK_PER_US;

        rmt_config(&rmt_channels[i]);
        rmt_set_rx_intr_en(rmt_channels[i].channel, true);
        rmt_rx_start(rmt_channels[i].channel, 1);
    }
 
  rmt_isr_register(rmt_isr_handler, NULL, 0, NULL);

  Serial.begin(115200); // USB serial (for DEBUG) 
 
  
}
 
void loop() 
{
    
  
  if (millis() - lastStateTime > 300)     // Print the data every 300ms
  {
    lastStateTime = millis();
    Serial.println(ReceiverChannels[0]);
    Serial.println(ReceiverChannels[1]);
    Serial.println(ReceiverChannels[2]);
    Serial.println(ReceiverChannels[3]);
    Serial.println(ReceiverChannels[4]);
    Serial.println(ReceiverChannels[5]);
 
   
    Serial.println("");}
 
  
  
}
