# BazenBot
Automatické ovládání bazénu na platformě arduino NANO

Tohle je mé první seznámení s arduinem a taky první projekt.

Díly:

1/ Arduino NANO

2/ RTC - DS3231

3/ LCD 16x2 / I2C

4/ 2x Thermometer DS18B20

5/ 4x Buttons

6/ modul 4 relay

7/ Motorized Ball Valve,DC5V/12V 1"

8/ 3x LED

9/ 1x DUO LED

10/ 4x rezistor 330Ω

11/ 1x resitor 4k7Ω

Photos of construction: 

http://rellik.rajce.idnes.cz/BazenBot/

Video:

https://www.youtube.com/watch?v=J7qZgJJ588w&t=6s

Funkce:

Automatika funguje ve dvou režimech:

1/ Brzo ráno a pozdě večer dle nastaveného času spouští filtraci a dávkuje chlor. Filtrace je nastavená na dobu jedné hodiny a dávka chloru dle průtoku čerpadla a velikosti bazénu na 40 sekund.
2/ Mimo předchozí nastavené filtrování hlídá teplotu solárního panelu a teplotu vody v bazénu. Pokud je teplota bazénu nízká (v kódu pod 24°C) a teplota soláru vyšší, tak zapne filtraci a přepne ventil pro ohřev. Na čidle teploty soláru je pak nastavená hysterze, která omezuje časté spínání čerpadla. Dále pak pokud je už teplota bazénu na požadované teplotě, tak se nic neděje a to i když je teplota soláru vyšší.


Dávka chloru:

Po zmáčknutí se přeruší automatika bazénu, zapne se filtrační čerpadlo a následně čerpadlo chloru. Po dávce chloru vypne čerpdalo po nasatveném časovém odstupu kdy přejde opět i celý systém do automatického režimu. Tento režim ještě není plně vychytaný a je nutné tuto činnost dávat dle hodin v 0-5 sekundě jakékoliv minuty.


Čištění/filtrace:

Po zmáčknutí tlačítka se vypne automatika, zapne filtrační čerpadlo a přepne ventil ohřevu pokud byl v poloze na ohřev. Tato činnost je dobrá pro promíchání vody v bazénu po aplikacji jiných chemikálií než je chlor. 

Pokud se má bazén čistit, je nutné zmáčknout tlačítko "OFF", čímž se systém vypne. V tuto dobu je možné si připravit věci potřebné k vysávání (čištění) bazénu. Pak stačí zmáčknout tlačítko "Čištění/filtrace" a vyčistit bazén. Po čištění opět vypnout (OFF), dát vše do původního stavu a opět zapnout automatiku, případně dát novou dávku chloru.
