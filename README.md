# Attiny84_PIR_Relay
Attiny84 based system that control a relay through a PIR sensor (with Arduino code).
[See Image of the board](Image.png).
-------------------------------------------------------------------------------------
Proceso de funcionamiento:  
  1- Mira la luminosidad del ambiente.
  2- Compara la luminosidad con cierto umbral.
  3- Si todavia hay luz, se duerme durante 10 minutos, tras ello se despierta y pasa a 1.
  4- Si hay suficiente oscuridad, detecta presencia.
  5- Si hay presencia, enciende la luz y se duerme durante 1 minuto, tras ello se despierta y pasa a 1.
  6- Si no hay presencia, se apaga la luz y se duerme durante 1 segundo, tras ello se despierta y pasa a 1.
  
Habra por tanto dos modos de funcionamiento según la luminosidad:
  De dia: Duerme y se despierta cada 10 minutos para ver si se ha hecho de noche.
  De noche: Duerme y se despierta cada 1 segundo para ver si hay presencia. Si hay presencia enciende la luz y se
  espera (durmiendo) 1 minuto. Si no hay presencia apaga la luz.
  
  Nota: Al programar la posicion del interruptor hay que tenerlo en cuenta para que no este en un estado de pull
  (activado) ya que es un pin de programacion.
  
Attiny84 1MHz
