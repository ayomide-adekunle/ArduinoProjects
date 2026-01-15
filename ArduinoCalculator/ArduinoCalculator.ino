#include <LiquidCrystal.h>
#include <Keypad.h>   // Look for Keypad by Mark Stanley, Alexander Brevig.

// ---------------- LCD PINS (UNO) ----------------
const uint8_t LCD_RS = 12;
const uint8_t LCD_E  = 11;
const uint8_t LCD_D4 = 10;
const uint8_t LCD_D5 = 9;
const uint8_t LCD_D6 = 8;
const uint8_t LCD_D7 = 7;

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// ---------------- KEYPAD SETUP ------------------
const byte ROWS = 4, COLS = 4;

// Key layout
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// UNO pins for keypad
// byte rowPins[ROWS] = {6, 5, 4, 3};     // R1..R4
// byte colPins[COLS] = {A0, A1, A2, A3}; // C1..C4
byte rowPins[ROWS] = {A3, A2, A1, A0};     // R1..R4
byte colPins[COLS] = {3, 4, 5, 6}; // C1..C4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------------- CALCULATOR STATE --------------
const uint8_t MAX_TOKENS = 16;  
long numbers[MAX_TOKENS];
char opsArr[MAX_TOKENS];        
uint8_t nCount = 0, oCount = 0;

String currentNumber = "";
String expr = "";

// --------------- Helpers -----------------------
void showTopTail(const String &s) {
  lcd.setCursor(0, 0);
  if (s.length() <= 16) {
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print(s);
  } else {
    String tail = s.substring(s.length() - 16);
    lcd.print(tail);
  }
}

void showBottom(const String &s) {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  if (s.length() <= 16) lcd.print(s);
  else lcd.print(s.substring(0, 16));
}

void clearAll() {
  currentNumber = "";
  expr = "";
  nCount = 0;
  oCount = 0;
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Arduino Calc");
  lcd.setCursor(0, 1); lcd.print("Enter digits...");
}

bool pushNumberIfAny() {
  if (currentNumber.length() == 0) return false;
  if (nCount >= MAX_TOKENS) return false;
  long v = currentNumber.toInt();
  numbers[nCount++] = v;
  currentNumber = "";
  return true;
}

void appendDigit(char d) {
  if (currentNumber.length() < 10) {
    currentNumber += d;
    expr += d;
  }
}

void appendOperator(char op) {
  if (currentNumber.length() == 0 && nCount == 0) return;
  pushNumberIfAny();
  if (nCount == oCount && oCount > 0) {
    opsArr[oCount - 1] = op;
    if (expr.length() > 0) expr.setCharAt(expr.length() - 1, op);
    return;
  }
  if (oCount >= MAX_TOKENS) return;
  opsArr[oCount++] = op;
  expr += op;
}

bool computeResult(long &out) {
  pushNumberIfAny();
  if (nCount == 0) return false;
  if (oCount >= nCount) {
    if (oCount > 0) oCount--;
    if (expr.length() > 0) expr.remove(expr.length() - 1);
  }
  if (oCount == 0) { out = numbers[0]; return true; }

  long n2[MAX_TOKENS];
  char o2[MAX_TOKENS];
  uint8_t n2c = 0, o2c = 0;

  n2[n2c++] = numbers[0];

  for (uint8_t i = 0; i < oCount; i++) {
    char op = opsArr[i];
    long rhs = numbers[i + 1];

    if (op == '*' || op == '/') {
      long lhs = n2[n2c - 1];
      if (op == '*') n2[n2c - 1] = lhs * rhs;
      else {
        if (rhs == 0) return false;
        n2[n2c - 1] = lhs / rhs;
      }
    } else {
      o2[o2c++] = op;
      n2[n2c++] = rhs;
    }
  }

  long acc = n2[0];
  for (uint8_t i = 0; i < o2c; i++) {
    long rhs = n2[i + 1];
    if (o2[i] == '+') acc += rhs;
    else              acc -= rhs;
  }
  out = acc;
  return true;
}

// --------------- Setup -------------------------
void setup() {
  lcd.begin(16, 2);

  // --- Show welcome message first ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome User!");
  lcd.setCursor(0, 1);
  lcd.print("Calc by Ayo :)");  // change this name
  delay(3000);  // show for 3 seconds

  // --- Then go into calculator mode ---
  clearAll();
}

// --------------- Main Loop ---------------------
void loop() {
  char k = keypad.getKey();
  if (!k) return;

  switch (k) {
    case '0'...'9':
      appendDigit(k);
      showTopTail(expr);
      showBottom(currentNumber.length() ? currentNumber : " ");
      break;

    case 'A': appendOperator('+'); showTopTail(expr); showBottom(" "); break;
    case 'B': appendOperator('-'); showTopTail(expr); showBottom(" "); break;
    case '*': appendOperator('*'); showTopTail(expr); showBottom(" "); break;
    case 'D': appendOperator('/'); showTopTail(expr); showBottom(" "); break;

    case 'C': clearAll(); break;

    case '#': {
      long result;
      bool ok = computeResult(result);
      showTopTail(expr);
      if (!ok) {
        showBottom("Err (e.g. /0)");
      } else {
        showBottom(String(result));
        numbers[0] = result;
        nCount = 1;
        oCount = 0;
        currentNumber = "";
        expr = String(result);
      }
      break;
    }
  }
}
