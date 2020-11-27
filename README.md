# デバイスドライバーの改造
## 使い方
1,git clone https://github.com/masakifujiwara1/myled.git  
2,cd myled  
3,make  
4,sudo insmod myled.ko  
5,sudo chmod 666 /dev/myled0  
6, echo (0~3) > /dev/myled0
## 内容
- echo 0 の時：LED消灯
- echo 1 の時：LED点灯
- echo 2 の時:モールス信号のSOS（・・・ －－－ ・・・）を再現したLEDの点滅
- echo 3 の時：LEDのpwm制御
## モールス信号について
- モールス信号の再現は無線局運用規則 別表第一号 モールス符号(第十二条関係)に則って再現しています。  
以下本文の一部抜粋  
符号の線及び間隔  
1　一線の長さは、三点に等しい。  
2　一符号を作る各線又は点の間隔は、一点に等しい。  
3　二符号の間隔は、三点に等しい。  
4　二語の間隔は、七点に等しい。  
出典：[総務省　電波利用ホームページ](https://www.tele.soumu.go.jp/horei/reiki_honbun/72393000001.html)  
## pwm制御について
- 4sかけてduty比が0%→100%→0%になるようになっています。
- 10msでduty比が0.5%増減→1sで50％ほど変化
## youtube
https://www.youtube.com/watch?v=1XYUYnG7E9o&ab_channel=%E5%8B%95%E7%94%BB%E4%BF%9D%E7%AE%A1%E5%BA%AB
