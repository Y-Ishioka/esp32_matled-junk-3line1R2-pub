◆概要

秋月ジャンクマトリクスLEDを使用した時計付きRSSニュース表示機

マトリクスLEDモジュールは１２セットを使用

１～４セットへ「月日」、５～８セットへ「時分秒」、９～１２セットへ「ＲＳＳニュース」を表示

◆参考資料

https://akizukidenshi.com/img/contents/kairo/%E3%83%87%E3%83%BC%E3%82%BF/%E8%A1%A8%E7%A4%BA%E8%A3%85%E7%BD%AE/LED%E9%9B%BB%E5%85%89%E6%8E%B2%E7%A4%BA.pdf

◆参考情報１

https://cba.sakura.ne.jp/kit01/kit_464.htm

◆参考情報２

https://x.com/YI_Studio/status/1842111760213663799

https://x.com/YI_Studio/status/1845173685524758563

https://x.com/YI_Studio/status/1845208811319308641

https://x.com/YI_Studio/status/1845302071962239189

https://x.com/YI_Studio/status/1900711287463834044

◆ピンアサイン

マトリクスLEDパネルと接続するGPIOは以下の通り。

　SIN1A     12

　SIN2A     13

　SIN3A     14

　CLOCK     15

　LATCH     16

　SIN1B     17

　SIN2B     18

　SIN3B     19

　SIN1C     25

　SIN2C     26

　SIN3C     27


また、表示の制御に使うGPIOは以下の通り。

・スライドの速度設定（必須）

　可変抵抗の電圧値をADCで取得し速度を決定

　VOL       34

・スライド表示の一時停止（省略可）

　BTN        4

・Wi-Fiデータ受信インジケーター（省略可）

　LED        2

◆必須設定

esp32_matled-junk-2line-pub.ino ファイルの20行目付近の以下の行へアクセスするWi-Fiの SSIDとパスフレーズを設定する必要があります。

```
const char* ssid1     = "xxxxxxxxxx";
const char* password1 = "xxxxxxxxxx";
const char* ssid2     = "xxxxxxxxxx";
const char* password2 = "xxxxxxxxxx";
const char* ssid3     = "xxxxxxxxxx";
const char* password3 = "xxxxxxxxxx";
```

◆回路図

![schem](https://github.com/Y-Ishioka/esp32_matled-junk-3line1R2-pub/blob/main/esp32_matled-junk-3line.jpg)

◆免責

　著作者は，提供するプログラムを用いることで生じるいかなる損害に関して，一切の責任を負わないものとします．

　たとえ，著作者が損害の生じる可能性を知らされていた場合も同様に一切の責任を負わないものとします．
