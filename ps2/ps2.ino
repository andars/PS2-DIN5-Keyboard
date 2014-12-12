/*
char keymap[] = {
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,'q',0,0,0,0,'z','s','a','w',
0,0,0,'c','x','d','e',0,0,0,
0,0,'v','f','t','r',0,0,0,'n',
'b','h','g','y',0,0,0,0,'m','j',
'u',0,0,0,0,0,'k','i','o',0,
0,0,0,0,0,'l',0,'p',0,0
};
*/
const int dataPin = 4;

uint16_t buffer[256];
int produceIndex = 0;
int consumeIndex = 0;

boolean released = false;

void setup() {
  Serial.begin(115200);  
  
  pinMode(dataPin, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(1, read, FALLING); 
}

void loop() {
  if (buffer[consumeIndex] > 0) {
    uint8_t data = (buffer[consumeIndex] >> 1) & 0xFF;
    Serial.write(data);
    
    buffer[consumeIndex] = 0;
    consumeIndex++;
  }
  if (consumeIndex >= 256) {  
    consumeIndex = 0;   
  }
}

void read() {
  static int bitCount = 0;
  static uint16_t data = 0x00;
  
  //each frame is 11 bits
  if (bitCount > 10) {
    bitCount = 0;
    buffer[produceIndex++] = data;
    data = 0x00;    
  } 
  
  if (produceIndex >= 256) {
    produceIndex = 0; 
  }
  data |= digitalRead(dataPin) << bitCount++;
  
}
