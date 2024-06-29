#include <Servo.h>

Servo sensorServo;  // Sensör için servo motor
Servo rudderServo;  // Dümen için servo motor

const int trigPin = 2;
const int echoPin = 3;
const int hedefLedPin = 11;
const int gorusAciLedPin = 12;
const int hedefeUlasmaLedPin = 13;
const int motorIn1Pin = 6;  // Motor sürücü IN1 pini
const int motorIn2Pin = 7;  // Motor sürücü IN2 pini
const int motorPWMPin = 5;  // Motor sürücü PWM pini

long sure;
int mesafe;

void setup() {
  sensorServo.attach(9);  // Sensör servo motoru 9 numaralı pine bağlandı
  rudderServo.attach(10);  // Dümen servo motorunu 10 numaralı pine bağlandı
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(hedefLedPin, OUTPUT);
  pinMode(gorusAciLedPin, OUTPUT);
  pinMode(hedefeUlasmaLedPin, OUTPUT);
  pinMode(motorIn1Pin, OUTPUT);
  pinMode(motorIn2Pin, OUTPUT);
  pinMode(motorPWMPin, OUTPUT);
  Serial.begin(9600);

  sensorServo.write(0);  // Sensör servo motorunu sıfırlandı
  rudderServo.write(90);  // Dümen servo motorunu merkezde tutuldu
  digitalWrite(hedefLedPin, LOW);  // LED'leri başta söndür
  digitalWrite(gorusAciLedPin, LOW);
  digitalWrite(hedefeUlasmaLedPin, LOW);
  delay(1000);  // Bir saniye bekle
}

void loop() {
  int hedefAci = -1;  // Hedef açıyı başlat
  int enKucukMesafe = 9999;  // Başlangıçta çok büyük bir değer ( Hocam burada en yakın kriterini en uzağa göre belirledim)
  bool hedefBulundu = false;  // Hedef bulundu bayrağı

  // Servo motoru 0 ile 150 derece arasında döndür
  for (int aci = 0; aci <= 150; aci += 1) {  // Daha küçük adımlarla hareket et
    sensorServo.write(aci);
    delay(100);  // Daha küçük adımlar için daha kısa gecikme
    mesafe = mesafeOlc();
    Serial.print("Açı: ");
    Serial.print(aci);
    Serial.print(" Mesafe: ");
    Serial.println(mesafe);

    if (mesafe <= 25) {  // Eğer mesafe 25 cm veya daha az ise
      digitalWrite(hedefLedPin, HIGH);  // Hedef algılandığında LED'i yak
      delay(200);
      digitalWrite(hedefLedPin, LOW);  // Hedef algılandığında LED'i söndür
      delay(200);
      
      if (mesafe < enKucukMesafe) {  // Eğer mesafe önceki mesafeden küçükse
        enKucukMesafe = mesafe;
        hedefAci = aci;
        hedefBulundu = true;
      }
    }
  }

  if (hedefBulundu) {  // Hedef açı bulunduysa hareket et
    Serial.println("En yakın hedef algılandı!");
    digitalWrite(gorusAciLedPin, HIGH);  // En yakın hedef bulunduğunda LED'i yak
    sensorServo.write(hedefAci);  // Radar servosunu hedef açısına çevir
    hedefeGit(hedefAci);
  } else {
    dur();
  }
}

int mesafeOlc() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  sure = pulseIn(echoPin, HIGH);
  mesafe = sure * 0.034 / 2;  // Mesafeyi santimetre olarak hesapla
  return mesafe;
}

void hedefeGit(int aci) {
  if (aci < 90) {
    sagaDon(aci);
  } else if (aci > 90) {
    solaDon(aci);
  } else {
    ileriGit();
  }

  while (mesafe > 5) {  // Hedefe yaklaşırken sürekli mesafeyi kontrol et
    mesafe = mesafeOlc();
    Serial.print("Hedefe kalan mesafe: ");
    Serial.println(mesafe);
    delay(500);  // Biraz bekle ve tekrar kontrol et

    if (mesafe <= 25) {  // Hedef görüş açısında olduğunda
      digitalWrite(gorusAciLedPin, HIGH);  // LED'i yak
      ileriGit();
    } else {
      digitalWrite(gorusAciLedPin, LOW);  // Hedef görüş açısından çıktığında LED'i söndür
    }

    if (digitalRead(hedefeUlasmaLedPin) == HIGH) {  // Eğer 13 numaralı LED yanıyorsa
      dur();
      break;
    }
  }

  if (mesafe <= 5) {  // Hedefe ulaşıldığında
    Serial.println("Hedefe ulaşıldı!");  // Hedefe ulaşıldığında mesaj ver
    digitalWrite(gorusAciLedPin, LOW);  // Hedefe ulaşıldığında LED'i söndür
    digitalWrite(hedefeUlasmaLedPin, HIGH);  // Hedefe ulaşıldığında LED'i yak
    dur();  // Motoru durdur
    delay(10000);  // Hedefe ulaştıktan sonra 10 saniye bekle
    digitalWrite(hedefeUlasmaLedPin, LOW);  // 10 saniye sonra LED'i söndür
  }
}

void ileriGit() {
  digitalWrite(motorIn1Pin, HIGH);
  digitalWrite(motorIn2Pin, LOW);
  analogWrite(motorPWMPin, 50);  // Düşük hızda ileri git
  Serial.println("İleri git");
}

void sagaDon(int aci) {
  int yeniAci = map(aci, 0, 90, 60, 90);  // Açı 0 ile 90 arasında sağa dönüş için haritalandı
  rudderServo.write(yeniAci);  // Dümeni sağa çevir
  Serial.print("Sağa dönüş açı: ");
  Serial.println(yeniAci);
  delay(1000);  // Küçük gecikme ile yavaşça hareket et
}

void solaDon(int aci) {
  int yeniAci = map(aci, 90, 180, 90, 120);  // Açı 90 ile 180 arasında sola dönüş için haritalandı
  rudderServo.write(yeniAci);  // Dümeni sola çevir
  Serial.print("Sola dönüş açı: ");
  Serial.println(yeniAci);
  delay(1000);  // Küçük gecikme ile yavaşça hareket et
}

void dur() {
  digitalWrite(motorIn1Pin, LOW);
  digitalWrite(motorIn2Pin, LOW);
  analogWrite(motorPWMPin, 0);  // Motoru durdur
  rudderServo.write(90);  // Dümeni merkezde tut
  Serial.println("Durdu ve dümen merkezde");
}
 