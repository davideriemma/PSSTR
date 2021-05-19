# Task Userspace

## Illustrazione del task

l'obbiettivo dell'esercizio è quello di creare un task real-time che controlla la presenza di tensione elettrica sulla linea di alimentazione di un circuito CCTV. Il taks ha un periodo di 10ms, entro il quale deve verificare che il dato letto da un adattatore da 220V a 3.3V indichi un livello alto. Se il livello è basso, deve scrivere un messaggio di alert in un buffer condiviso e settare un flag. Tali dati verrano poi letti dal main e stampati a schermo.
L'alimentazione viene a mancare dopo un tempo compreso tra 100ms e 125ms dall'inizio dell'essecuzione del task periodico.