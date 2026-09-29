RM = rm -rfv

SRC +=		\
main.c		\
print.c

OBJ +=		\
obj/main.o	\
obj/print.o

OUTPUT_FILE =	\
obj/Telemetre.elf

OUTPUT_FILE_NAME =\
Telemetre

HEX =	\
Telemetre.hex

FLAGS +=	\
-x c -funsigned-char -funsigned-bitfields -DDEBUG

FLAGS_CLASSIQUE +=							\
-Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums	\
-g2 -Wall -mmcu=atmega328p -B atmega328p -c -std=gnu99

./obj/%.o: ./%.c
	@echo
	@echo Building: $<
	# avr-gcc -Os -g -mmcu=atmega328p -I/usr/lib/avr ./$(SRC)
	avr-gcc $(FLAGS) -I/usr/lib/avr $(FLAGS_CLASSIQUE) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -o "$@" "$<"
	@echo Finished_building: $<

.PHONY: all spec dude clean

all: $(OUTPUT_FILE)

$(OUTPUT_FILE): $(OBJ)
	@echo
	@echo Building_target: $@
	avr-gcc -o$(OUTPUT_FILE) $(OBJ) -Wl,-Map="obj/USART.map" -Wl,--start-group -Wl,-lm -Wl,-lprintf_flt  -Wl,--end-group -Wl,--gc-sections -mmcu=atmega328p -B atmega328p -Wl,-u,vfprintf
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "obj/$(OUTPUT_FILE_NAME).elf" "$(OUTPUT_FILE_NAME).hex"
	@echo Finished_building_target: $@
	@echo
	# avr-objcopy -O ihex -R .eeprom a.out $(HEX)
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "obj/$(OUTPUT_FILE_NAME).elf" "obj/$(OUTPUT_FILE_NAME).hex"

spec:
	avr-objcopy -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex "obj/$(OUTPUT_FILE_NAME).elf" "obj/$(OUTPUT_FILE_NAME).eep" || exit 0
	avr-objdump -h -S "obj/$(OUTPUT_FILE_NAME).elf" > "obj/$(OUTPUT_FILE_NAME).lss"
	avr-objcopy -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "obj/$(OUTPUT_FILE_NAME).elf" "obj/$(OUTPUT_FILE_NAME).srec"
	avr-size "obj/$(OUTPUT_FILE_NAME).elf"

dude:
	avrdude -C /etc/avrdude.conf -c arduino -p atmega328p -D wiring -P /dev/ttyACM0 -b 115200 -U flash:w:$(HEX):a


clean:
	$(RM) $(HEX)
	$(RM) obj/*
	$(RM) $(OUTPUT_FILE)
