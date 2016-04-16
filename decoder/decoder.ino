/*
 * Sam Stein
 * Thomas Skinner
 * 
 * UWF ECE Dept
 * Digital Architecutre, Spring 2016
 * 
 * 16 Bit Thumb Instruction Set Decoder
 */

/*
 * Note that by the nature of counting boolean values (MSB being most left bit)
 * and array indexing (item 0 is left most in an array) that our arrays holding 
 * the instructions will be inverted images of the actual instruction in code, but 
 * will be entered by the user and displayed to the screen as if the first value
 * in the array will be the MSB even though in code it will be at index 0. So, when
 * reviewing this, treat pinsOut[0] as bit 15, pinsOut[1] as bit 14 and so on to 0.
 */

#include <LiquidCrystal.h>
LiquidCrystal lcd(38,39,40,41,42,43);

//int pins[16] = {
//  22,23,24,25,
//  26,27,28,29,
//  30,31,32,33,
//  34,35,36,37};

int pins[16] = {
  37,36,35,34,
  33,32,31,30,
  29,28,27,26,
  25,24,23,22};

void setup() {
  setPins(pins);
  setupLCD();
  testLEDs(pins);
  clearLCD();
}

void loop() {
  Serial.begin(9600);
  getInstruction(pins);
  clearLCD();
  displayInstruction(pins);
  decodeInstruction(pins);
  Serial.end();
}

/*
 *initialize a set of pins for output
 */
void setPins(int *inPins) {
  for (int i = 0 ; i < 16 ; i++) {
    pinMode(inPins[i], OUTPUT);
  }
}

/*
 * Prompt user for instruction bit vector
 */
void getInstruction(int *inPins) {
  for (int i = 0 ; i < 16 ; i++) {
    while (Serial.available() == 0);
    char charRead = Serial.read();
    if ( charRead == '1')
    {
      digitalWrite(inPins[i], HIGH);
    }
    else
    {
      digitalWrite(inPins[i], LOW);
    }
  }
}

/*
 * Display the instruction bit vector entered by the user
 */
void displayInstruction(int *inPins) {
  for (int i = 0 ; i < 16 ; i++) {
    if (digitalRead(inPins[i]) == HIGH) {
      Serial.print(1); 
      Serial.print(" ");
    }
    else {
      Serial.print(0);
      Serial.print(" ");
    }
  }
  Serial.println();
}

void testLEDs ( int *inPins )
{
  for ( int i = 0; i < 16; i++ )
  {
    digitalWrite(inPins[i], HIGH);
    delay(150);
    digitalWrite(inPins[i], LOW);
    delay(10);
  }
  for ( int i = 0; i < 16; i++ )
  {
    digitalWrite(inPins[i], HIGH);
  }
  delay(400);
  for ( int i = 0; i < 16; i++ )
  {
    digitalWrite(inPins[i], LOW);
  }
}

void setupLCD ( void ) 
{
  for ( int i = 2; i <=7; i++ )
    pinMode ( i, OUTPUT );
  analogWrite(2, 0);
  lcd.begin(16, 2);
  lcd.print("Testing");
}

void clearLCD ( void )
{
  lcd.clear();
}

void nextLine( void )
{
  lcd.setCursor(0,1);
}

/*
 * Decode instruction (just going to do Format 1: move shifted register for now)
 * Note - in the final project, this function will be taking pinsIn which will 
 *        be read from our circuit, however, for testing purposes pinsOut should do
 */
void decodeInstruction(int *inPins) {
  //reverse instruction from what was read so that we can process it human-like
  int in[16];
  for (int i = 0 ; i < 16 ; i++) {
    if (digitalRead(inPins[i]) == HIGH) {
      in[15-i] = HIGH;
    }
    else {
      in[15-i] = LOW;
    }
  }
  
  
  //determine the function
  int func = bit(2)*in[15] + bit(1)*in[14] + bit(0)*in[13];
  switch ( func )
  {
    case 0:
      //f 1 2
      f1thru2 ( in );
      break;
    case 1:
      //f 3
      f3 ( in );
      break;
    case 2:
      //f 4 5 6 7 8
      f4thru8( in );
      break;
    case 3:
      //f 9
      f9 ( in );
      break;
    case 4:
      //f 10 11
      f10thru11 ( in );
      break;
    case 5:
      //f 12 13 14
      f12thru14 ( in );
      break;
    case 6:
      //f 15 16 17
      f15thru17( in );
      break;
    case 7:
      //f 18 19
      f18thru19( in );
      break;
    default:
      Serial.println("An error occurred when finding the function.");
      lcd.print("ERR");
  }
}

void f1thru2 ( int *in )
{
  int test = bit(1)*in[12] + bit(0)*in[11];
  if ( test == 3 )
    addSub ( in );
  else
    moveShifted ( in );
}

void f3 ( int *in )
{
  immMCAS ( in );
}

void f4thru8 ( int *in )
{
  if ( !in[12] )
    if ( !in[11] )
      if ( !in[10] )
        aluOps ( in );
      else
        hiRegOps ( in );
    else
      PCRelLoad ( in );
  else
    if ( !in[9] )
      loadStoreRegOffset ( in );
    else
      loadStoreSignExt ( in );
}

void f9 ( int *in )
{
  loadStoreImm ( in );
}

void f10thru11 ( int *in )
{
  if ( !in[12] )
    loadStoreHalf ( in );
  else
    loadStoreSPRel ( in );
}

void f12thru14 ( int *in )
{
  if ( !in[12] )
    loadAddr ( in );
  else
    if ( !in[10] )
      addOffsetSP ( in );
    else
      pushPop ( in );
}

void f15thru17 ( int *in )
{
  if ( !in[12] )
    multLoadStore ( in );
  else
    if ( !( in[11] && in[10] && in[9] && in[10] ) )
      condBranch ( in );
    else
      softwareInt ( in );
}

void f18thru19 ( int *in )
{
  if ( !in[12] )
    uncondBranch ( in );
  else
    longBranch ( in );
}

void moveShifted ( int *in )
{
  int op = bit(1)*in[12] + bit(0)*in[11];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];
  int rs = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int offset = bit(4)*in[10] + bit(3)*in[9] + bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  switch ( op )
  {
    case 0:
      lcd.print("LSL R");
      break;
    case 1:
      lcd.print("LSR R");
      break;
    case 2:
      lcd.print("ASR R");
      break;
    default:
      Serial.println("problem decoding in addSub()");
      lcd.print("ERR");
      exit(-1);
  }
  lcd.print(rd);
  lcd.print(", R");
  lcd.print(rs);
  lcd.print(", #");
  lcd.print(offset);
}

void addSub ( int *in )
{
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];
  int rs = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rn_off = bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  if ( !in[9] )
    lcd.print("ADD ");
  else
    lcd.print("SUB ");
  lcd.print ( "R" );
  lcd.print ( rd );
  lcd.print ( ", R" );
  lcd.print ( rs );
  lcd.print ( ", " );
  if ( !in[10] )
    lcd.print ( "R" );
  else
    lcd.print( "#" );
  lcd.print ( rn_off );
}

void immMCAS ( int *in )
{
  int offset8 = 0;
  int op = bit(1)*in[12] + bit(0)*in[11];
  int rd = bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];
  for (int i = 7 ; i > -1 ; i--) offset8 = offset8 + bit(i)*in[i];

  switch (op)
  {
    case 0:
      lcd.print("MOV R");
    break;
    case 1:
      lcd.print("CMP R");
    break;  
    case 2:
      lcd.print("ADD R");
    break;
    case 3:
      lcd.print("SUB R");
    break;
    default:
      Serial.println("Error in decoding move/compare/add/subtract immediate");
      lcd.print("ERR");
      exit(-1);
  }
  
    lcd.print(rd);
    lcd.print(", #");
    lcd.print(offset8);
}

void aluOps ( int *in )
{
  int op = bit(3)*in[9] + bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  int rs = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];
  
  switch (op)
  {
    case 0:
      lcd.print("AND R");
    break;
    case 1:
      lcd.print("EOR R");
    break;
    case 2:
      lcd.print("LSL R");
    break;
    case 3:
      lcd.print("LSR R");
    break;
    case 4:
      lcd.print("ASR R");
    break;
    case 5:
      lcd.print("ADC R");
    break;
    case 6:
      lcd.print("SBC R");
    break;
    case 7:
      lcd.print("ROR R");
    break;
    case 8:
      lcd.print("TST R");
    break;
    case 9:
      lcd.print("NEG R");
    break;
    case 10:
      lcd.print("CMP R");
      break;
    case 11:
      lcd.print("CMN R");
      break;
    case 12:
      lcd.print("ORR R");
      break;
    case 13:
      lcd.print("MUL R");
      break;
    case 14:
      lcd.print("BIC R");
      break;
    case 15:
      lcd.print("MVN R");
      break;
    default:
      Serial.println("Error decoding aluOps");
      lcd.print("ERR");
      exit(-1);
  }

  lcd.print(rd);
  lcd.print(", R");
  lcd.print(rs);  
}

void hiRegOps ( int *in )
{
  int op = bit(1)*in[9] + bit(0)*in[8];
  int h1 = in[7];
  int h2 = in[6];
  int rs = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];

  switch (op)
  {
    case 0:
      lcd.print("ADD ");
    break;
    case 1:
      lcd.print("CMP ");
    break;
    case 2:
      lcd.print("MOV ");
    break;
    case 3:
      lcd.print("BX ");
    break;
    default:
      Serial.println("Error in decoding Hi register operations/branch exchange");
      lcd.print("ERR");
      exit(-1);
  }
  if ( op != 3 )
  {
    if ( h1 )
      lcd.print("H");
    else
      lcd.print("R");
    lcd.print(rd);
    lcd.print(", ");
  }
  if ( h2 )
    lcd.print("H");
  else
    lcd.print("R");
  lcd.print(rs);
}

void PCRelLoad ( int *in )
{
  int word8 = 0;
  int rd = bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];
  for (int i = 7 ; i > -1 ; i--) word8 = word8 + bit(i+2)*in[i];

  lcd.print("LDR R");
  lcd.print(rd);
  lcd.print(", ");
  nextLine();
  lcd.print("[PC, #");
  lcd.print(word8);
  lcd.print("]");
}

void loadStoreRegOffset ( int *in )
{
  int lb = bit(1)*in[11] + bit(0)*in[10];
  int ro = bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  int rb = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];

  switch (lb)
  {
    case 0:
      lcd.print("STR R");
    break;
    case 1:
      lcd.print("STRB R");
    break;
    case 2:
      lcd.print("LDR R");
    break;
    case 3:
      lcd.print("LDRB R");
    break;
    default:
      Serial.println("Error decoding load/store with register offset");
      lcd.print("ERR");
      exit(-1);
  }

  lcd.print(rd);
  lcd.print(", ");
  if ( lb % 2 ) nextLine();
  lcd.print("[R");
  lcd.print(rb);
  lcd.print(", R");
  lcd.print(ro);
  lcd.print("]");
  
}

void loadStoreSignExt ( int *in )
{
  int hs = bit(1)*in[10] + bit(0)*in[11];
  int ro = bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  int rb = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];

  switch (hs)
  {
    case 0:
      lcd.print("STRH R");
    break;
    case 1:
      lcd.print("LDRH R");
    break;
    case 2:
      lcd.print("LDSB R");
    break;
    case 3:
      lcd.print("LDSH R");
    break;
    default:
      Serial.println("Error decoding load/store sign-extended byte/halfword");
      lcd.print("ERR");
      exit (-1);
  }

  lcd.print(rd);
  lcd.print(", ");
  nextLine();
  lcd.print("[R");
  lcd.print(rb);
  lcd.print(", R");
  lcd.print(ro);
  lcd.print("]");
}

void loadStoreImm ( int *in )
{
  int bl = bit(1)*in[12] + bit(0)*in[11];
  int offset5 = bit(4)*in[10] + bit(3)*in[9] + bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  int rb = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];

  if ( !in[12] ) offset5 *= 4;

  switch (bl)
  {
    case 0:
      lcd.print("STR R");
    break;
    case 1:
      lcd.print("LDR R");
    break;
    case 2:
      lcd.print("STRB R");
    break;
    case 3:
      lcd.print("LDRB R");
    break;
    default:
      Serial.println("Error decoding load/store with immediate offset");
      lcd.print("ERR");
      exit (-1);
  }

  lcd.print(rd);
  lcd.print(", ");
  nextLine();
  lcd.print("[R");
  lcd.print(rb);
  lcd.print(", #");
  lcd.print(offset5);
  lcd.print("]");
}

void loadStoreHalf ( int *in )
{
  int offset5 = bit(4)*in[10] + bit(3)*in[9] + bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  int rb = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];
  offset5 *= 2;

  if (in[11] == 0) lcd.print("STRH R");
  else lcd.print("LDRH R");

  lcd.print(rd);
  lcd.print(", ");
  nextLine();
  lcd.print("[R");
  lcd.print(rb);
  lcd.print(", #");
  lcd.print(offset5);
  lcd.print("]");
}

void loadStoreSPRel ( int *in )
{
  int word8 = 0;
  int rd = bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];
  for (int i = 7 ; i > -1 ; i--) word8 = word8 + bit(i)*in[i];
  word8 *= 4;

  if (in[11] == 0) lcd.print("STR R");
  else lcd.print("LDR R");

  lcd.print(rd);
  lcd.print(", ");
  nextLine();
  lcd.print("[SP, #");
  lcd.print(word8);
  lcd.print("]");
}

void loadAddr ( int *in )
{
  int word8 = 0;
  int rd = bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];
  for (int i = 7 ; i > -1 ; i--) word8 = word8 + bit(i)*in[i];
  word8 *= 4;

  lcd.print("ADD R");
  lcd.print(rd);
  lcd.print(", ");
  if (in[11] == 0) lcd.print("PC, #");
  else lcd.print("SP, ");
  nextLine();
  lcd.print("#");
  lcd.print(word8);
}

void addOffsetSP ( int *in )
{
  int sWord7 = 0;
  for (int i = 6 ; i > -1 ; i--) sWord7 = sWord7 + bit(i)*in[i];
  sWord7 *= 4;

  lcd.print("ADD SP, #");
  if (in[7] == 1) lcd.print("-");
  lcd.print(sWord7);
}

void pushPop ( int *in )
{
  int flag = 0;
  int lr = bit(1)*in[11] + bit(0)*in[8];
  int sum = 0;
  int count = 0;
  
  for ( int i = 7; i > -1; i-- )
  {
    sum += in[i];
  }
  if ( sum == 0 )
    Serial.println("Invalid Instruciton. No registers defined.");
  else
  {
    switch (lr)
    {
      case 0:
        lcd.print("PUSH {");
        for (int i = 7 ; i > -1 ; i--) {
          flag = 0;
          if (in[i] == 1) {
            count++;
            lcd.print("R");
            lcd.print(i);
            for (int j = i-1 ; j > -1 ; j--) if (in[j] == 1) flag = 1;
            if (flag == 1) lcd.print(",");
            //else break;
          }
          if ( count == 3 && sum > 3 ) nextLine();
        }
        lcd.print("}");
      break;
      
      case 1:
        lcd.print("PUSH{");
        for (int i = 7 ; i > -1 ; i--) {
          if (flag == 1) lcd.print(",");
          flag = 0;
          if (in[i] == 1) {
            count++;
            lcd.print("R");
            lcd.print(i);
            for (int j = i-1 ; j > -1 ; j--) if (in[j] == 1) flag = 1;
            //else break;
          }
          if ( count == 3 && sum >= 3) nextLine();
        }
        lcd.print(",LR}");
      break;
  
      case 2:
        lcd.print("POP {");
        for (int i = 7 ; i > -1 ; i--) {
          flag = 0;
          if (in[i] == 1) {
            count++;
            lcd.print("R");
            lcd.print(i);
            for (int j = i-1 ; j > -1 ; j--) if (in[j] == 1) flag = 1;
            if (flag == 1) lcd.print(",");
            //else break;
          }
          if ( count == 3 && sum > 3 ) nextLine();
        }
        lcd.print("}");
      break;
  
      case 3:
        lcd.print("POP{");
        for (int i = 7 ; i > -1 ; i--) {
          flag = 0;
          if (in[i] == 1) {
            count++;
            lcd.print("R");
            lcd.print(i);
            for (int j = i-1 ; j > -1 ; j--) if (in[j] == 1) flag = 1;
            if (flag == 1) lcd.print(",");
            //else break;
          }
          if ( count == 4 && sum >= 4 ) nextLine();
        }
        lcd.print(",PC}");
      break;
      default:
        Serial.println("Error decoding push/pop registers");
        lcd.print("ERR");
        exit (-1);  
    }
  }
}

void multLoadStore ( int *in )
{
  int flag;
  int rb = bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];

  if (in[11] == 0) lcd.print("STMIA R");
  else lcd.print("LDMIA R");

  lcd.print(rb);
  lcd.print("!, {");
 
  for (int i = 7 ; i > -1 ; i--) {
    flag = 0;
    if (in[i] == 1) {
      lcd.print("R");
      lcd.print(i);
      for (int j = i-1 ; j > -1 ; j--) if (in[j] == 1) flag = 1;
      if (flag == 1) lcd.print(", ");
      else break;
    }
  }

  lcd.print("}");
}

void condBranch ( int *in )
{
  int cond = bit(3)*in[11] + bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];
  int sOffset8 = 0;
  for (int i = 6 ; i > -1 ; i--) sOffset8 = sOffset8 + bit(i+1)*in[i];

  switch (cond)
  {
    case 0:
      lcd.print("BEQ ");
    break;
    case 1:
      lcd.print("BNE ");
    break;
    case 2:
      lcd.print("BCS ");
    break;
    case 3:
      lcd.print("BCC ");
    break;
    case 4:
      lcd.print("BMI ");
    break;
    case 5:
      lcd.print("BPL ");
    break;
    case 6:
      lcd.print("BVS ");
    break;
    case 7:
      lcd.print("BVC ");
    break;
    case 8:
      lcd.print("BHI ");
    break;
    case 9:
      lcd.print("BLS ");
    break;
    case 10:
      lcd.print("BGE ");
    break;
    case 11:
      lcd.print("BLT ");
    break;
    case 12:
      lcd.print("BGT ");
    break;
    case 13:
      lcd.print("BLE ");
    break;
    default:
      Serial.println("Error decoding conditional branch");
      lcd.print("ERR");
      exit (-1);
  }
  lcd.print("");
  if ( in[7] ) sOffset8 = (sOffset8*-1) - 2;
  lcd.print(sOffset8);
}

void softwareInt ( int *in )
{
  int value8 = 0;
  for (int i = 7 ; i > -1 ; i--) value8 = value8 + bit(i)*in[i];

  lcd.print("SWI ");
  lcd.print(value8);
}

void uncondBranch ( int *in )
{
  int value11 = 0;
  for ( int i = 9; i > -1; i--) value11 += bit(i+1)*in[i];
  lcd.print("B ");
  if ( in[10] ) value11 = (value11*-1) - 2;
  lcd.print(value11);
}

void longBranch ( int *in )
{
  lcd.print("BL label");
}

