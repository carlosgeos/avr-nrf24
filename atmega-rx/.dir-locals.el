;;; Directory Local Variables
;;; For more information see (info "(emacs) Directory Variables")

((c-mode
  (flycheck-gcc-args .
		     ("-mmcu=atmega328p" "-Os" "-Wall"))
  (flycheck-c/c++-gcc-executable . "/usr/bin/avr-gcc")))
