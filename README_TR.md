Author: Safa Demirhan
Number: 171101070
Course: BIL461


# Compile: #
gcc paging_seg.c -o seg

# Run: #
./seg executable_file reference_string.txt

executable_file: /usr/bin/gcc, a.out, hello
reference_string.txt: Iki sutundan olusan ilk sutunu segment numarasi,
ikinci sutunu page numarasi olan bir txt dosyasi. Program spesifik.

# Program nasil calisir ve varsayimlar: #

Program oncelikle linux komutu olan size komutunu C uzerinden cagirir ve ciktisini
size.txt adinda bir dosyaya aktarir. Daha sonra bu dosyayi okuyarak icerisinden 3 sayi
alir. Bunlar: text, data ve bss olmak uzere 3 segment'in kac bayt olduklaridir.
Bunlari 1024'e bolerek sayfa sayisi elde edilir. Daha sonra segment'lerin kac bayt oldugu,
hangi segment'te kac page oldugu ve segment basina kac physical frame
oldugu tablo olarak print edilir.

Daha sonra tercihe gore reference string dosyasini program kendi olusturabilir 
(yorum satirina aldim, ilgili segment'in segment numarasi bir bosluk page numarasi
olmak uzere olusturuyordu dosyayi, yerellik olmadigi icin de bulma orani cok dusuyordu)
ve asagidakine benzer bir reference string dosyasi olusturur program:

0 0
0 1
0 2
0 3
...
1 0
1 1 
...
2 0

Bunun yerine test etmek icin kendi reference string dosyanizi da programla ayni
dizinde bulunacak sekilde command line argument olarak verebilirsiniz.

Sonra, TLB ve Bellek adinda iki struct dizi olusturulur. 
TLBSIZE = 50, BELLEKSIZE = 100 olarak default define edilmistir.
set_bellek() fonk. ile bellegin icine su segment-page ikilileri konur:
(0,0),(1,0),(2,0)

Son olarak program reference string'i for ile doner ve sirasiyla TLB, Bellek,
Sanal bellekte olup olmadigini kontrol eder. TLB ve bellekte bulamadiysa sanal bellekte
kesin olarak bulur. isInTLB() fonk. TLB'de olup olmadigini soyler. Varsa, if'in icine
girer ve gerekli sleep ve delay'i ayarlar. Veri bulundugu icin TLB hit olmustur,
bunun icin var olan degiskeni 1 artirir. count_tlb() ile TLB'deki elemanlarin
timestamp'lerini 1 artirir ve TLB'de buldugu icin bellek ve sanal bellekte arama
yapmaz continue ile donmeye devam eder.
isInBellek() fonk. ile aranan verinin bellekte olup olmadigini kontrol eder ve
dogruysa if'in icine girip gerekli gecikmeleri ve hit/miss degiskenlerini artirir.
count_bellek() bellekteki verilerin timestamp'lerini 1 artirir. update_tlb() bellekte
buldugu verinin TLB'ye de tasinmasini saglar ki bir dahakine TLB'den eriselim.
else kisminda ise bellekte de bulamadigi icin sanal bellekten veriyi alir, gecikmeler
ve hit/miss degiskenleri artirilir. Veri sanal bellekte bulununca hem bellege hem 
TLB'ye bu veri kopyalanir.

Son olarak az once bahsi gecen degiskenler uzerinden TLB ve bellek icin bulamama
oranlari hesaplanir, toplam delay yazdirilir ve invalid reference sayisi print
edilir.

