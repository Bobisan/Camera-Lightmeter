    #include <LCD-I2C.h>
    #include <Wire.h>
    #include <SparkFun_VEML7700_Arduino_Library.h> // Include the SparkFun VEML7700 library
    // buttons : ISO = 9, SP = 10, A = 13, Contrast = 7, plus + 10 = 1, minus -10 = 8
    VEML7700 veml; // Create a VEML7700 object
    float arr_full_seconds_SP[] = { 30, 25, 20, 15, 13, 10, 8, 6, 5, 4, 3, 2.5, 2, 1.6, 1.3, 1};
    float arr_parts_seconds_SP[] = {1.3, 1.6, 2, 2.5, 3, 4, 5, 6, 8, 10, 13, 16, 20, 25, 30, 40, 50, 60, 80, 100, 125, 160, 200, 250, 320, 400, 500, 640, 800, 1000, 1250, 1600, 2000, 2500, 3200, 4000};
    float arr_Apreture[] = {1, 1.1, 1.2, 1.4, 1.6, 1.8, 2, 2.2, 2.4, 2.8, 3.2, 3.5, 4, 4.5, 5, 5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 25, 29};
    int SP_counter = 32;
    int Apreture_counter = 9;
    int Contrast = 130;
    LCD_I2C lcd(0x27, 16, 2); 
    float ISO_value = 100;
    float A_value = 2.8,SP_value = 60;
    int ISO_button = 9, SP_button = 10, A_button = 13, Contrast_button = 7, plus_button = 6, minus_button = 8;
    int ISO_button_state = 0, SP_button_state = 0, A_button_state = 0, Contrast_button_state = 0, plus_button_state = 0, minus_button_state = 0, Last_plus_button_state = 0, Last_minus_button_state = 0;
    int choose_const = -1, change_value = -1, go_back_to_normal = 0, Last_ISO = 0, Last_SP = 0, Last_A = 0, Last_SP_value = 0, Last_ISO_value = 0, Last_A_value = 0, ISO_value_for_calc = 0, Last_ISO_value_for_calc = 0;
    float lux = 0, SP_value_for_calc = 0, Last_SP_value_for_calc = 0, A_value_for_calc = 0, Last_A_value_for_calc = 0;
    void Parameter_menu(){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ISO");
        lcd.setCursor(5, 0);
        lcd.print("SP");
        lcd.setCursor(13, 0);
        lcd.print("A");
        lcd.setCursor(0, 1);
        lcd.print(ISO_value, 0);
        if(SP_counter > -1 && SP_counter < 16){
            SP_value = arr_full_seconds_SP[SP_counter];
            lcd.setCursor(5, 1);
            lcd.print(SP_value, 1);
        }
        if(SP_counter > 15 && SP_counter < 53){
            SP_value = arr_parts_seconds_SP[SP_counter - 15];
            lcd.setCursor(4, 1);
            lcd.print("1/");
            lcd.setCursor(6, 1);
            lcd.print(SP_value, 1);
        }
        lcd.setCursor(12, 1);
        lcd.print(A_value);
    }
    void ISO_CHANGE_double() {
        int NewISO = ISO_value*2;
        if(NewISO < 10000){
        ISO_value = NewISO;
        lcd.setCursor(0, 1);
        lcd.print("    ");
        lcd.setCursor(0, 1);
        lcd.print(ISO_value, 0);
        }

    }
    void ISO_CHANGE_decrease(){
        int NewISO = ISO_value*0.5;
        if(NewISO >= 25){
            ISO_value = NewISO;
            lcd.setCursor(0, 1);
            lcd.print("    ");
            lcd.setCursor(0, 1);
            lcd.print(ISO_value, 0);
        }
    }
    int SP_CHANGE(){
        if(SP_counter > -1 && SP_counter <= 52){
            lcd.setCursor(4, 1);
            lcd.print("        ");
            if(SP_counter > -1 && SP_counter < 16){
                SP_value = arr_full_seconds_SP[SP_counter];
                SP_value_for_calc = SP_value;
                lcd.setCursor(5, 1);
                lcd.print(SP_value, 1);
            }
            if(SP_counter > 15 && SP_counter < 53){
                SP_value = arr_parts_seconds_SP[SP_counter - 15];
                SP_value_for_calc = 1/SP_value;
                lcd.setCursor(4, 1);
                lcd.print("1/");
                lcd.setCursor(6, 1);
                lcd.print(SP_value, 1);
            }
        }
    }

    void setup()
    {
        Serial.begin(9600); // Start serial communication
        Wire.begin(); // Begin I2C communication
        analogWrite(6, Contrast);// Set contrast for the LCD
        lcd.begin(&Wire); // Initialize the LCD (16x2)
        lcd.backlight();
        lcd.display();
        lcd.setCursor(0, 0);
        lcd.print("Choose parameter:");
        lcd.setCursor(2, 1);
        lcd.print("ISO");
        lcd.setCursor(7, 1);
        lcd.print("SP");
        lcd.setCursor(13, 1);
        lcd.print("A");
        //lcd.blink();
        pinMode(ISO_button, INPUT);
        pinMode(SP_button, INPUT);
        pinMode(A_button, INPUT);
        pinMode(Contrast_button, INPUT);
        pinMode(plus_button, INPUT);
        pinMode(minus_button, INPUT);
        // Initialize the VEML7700 sensor
        if (veml.begin() == false)
        {
            Serial.println("Unable to communicate with the VEML7700. Please check the wiring. Freezing...");
            while (1)
                ;
        }
    }

    void loop()
    {
        if(go_back_to_normal){
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Choose parameter:");
            lcd.setCursor(2, 1);
            lcd.print("ISO");
            lcd.setCursor(7, 1);
            lcd.print("SP");
            lcd.setCursor(13, 1);
            lcd.print("A");
            go_back_to_normal = 0;
            choose_const = -1;
            change_value = -1;
        }
        while(choose_const == -1){
            ISO_button_state = digitalRead(ISO_button);
            SP_button_state = digitalRead(SP_button);
            A_button_state = digitalRead(A_button);
            if(Last_ISO != ISO_button_state){
                if (ISO_button_state == HIGH) {
                choose_const = 1;
                Last_ISO = ISO_button_state;
                Parameter_menu();
            }
            }
            if(Last_SP != SP_button_state){
                if (SP_button_state == HIGH){
                choose_const = 2;
                Last_SP = SP_button_state;
                Parameter_menu();
            }
            }
            if(Last_A != A_button_state){
                if(A_button_state == HIGH){
                choose_const = 3;
                Last_A = A_button_state;
                Parameter_menu();
            }
            }
            Last_ISO = ISO_button_state;
            Last_SP = SP_button_state;
            Last_A = A_button_state;
        }
        switch(choose_const){
            case 1 : 
                while(change_value != 1){
                    ISO_button_state = digitalRead(ISO_button);
                    SP_button_state = digitalRead(SP_button);
                    A_button_state = digitalRead(A_button);
                    if(Last_ISO != ISO_button_state){
                        if(ISO_button_state == HIGH){
                            change_value = 1;
                        }
                    }   
                    if(Last_SP != SP_button_state){
                        if(SP_button_state == HIGH){
                            change_value = 2;
                        }
                    }
                    if(Last_A != A_button_state){
                        if(A_button_state == HIGH){
                            change_value = 3;
                        }
                    }
                    switch(change_value){
                        case 1:
                            go_back_to_normal = 1;
                            break;
                        case 2:
                            plus_button_state = digitalRead(plus_button);
                            minus_button_state = digitalRead(minus_button);
                            if(Last_plus_button_state != plus_button_state){
                            if(plus_button_state == HIGH){
                                SP_counter = SP_counter + 1;
                                SP_CHANGE();
                            }
                        }
                            if(Last_minus_button_state != minus_button_state){
                                if(minus_button_state == HIGH){
                                SP_counter = SP_counter - 1;
                                SP_CHANGE();
                            }
                        }
                            Last_plus_button_state = plus_button_state;
                            Last_minus_button_state = minus_button_state;
                            break;
                        case 3:
                            plus_button_state = digitalRead(plus_button);
                            minus_button_state = digitalRead(minus_button);
                            if(Last_plus_button_state != plus_button_state){
                            if(plus_button_state == HIGH){
                                if(Apreture_counter + 1 < 29){
                                    Apreture_counter++;
                                    A_value = arr_Apreture[Apreture_counter];
                                    lcd.setCursor(12, 1);
                                    lcd.print(A_value, 2);
                                }
                            }
                        }
                            if(Last_minus_button_state != minus_button_state){
                                if(minus_button_state == HIGH){
                                    if(Apreture_counter - 1 > -1){
                                        Apreture_counter--;
                                        A_value = arr_Apreture[Apreture_counter];
                                        lcd.setCursor(12, 1);
                                        lcd.print(A_value, 2);
                                    }
                                }
                            }
                            Last_plus_button_state = plus_button_state;
                            Last_minus_button_state = minus_button_state;
                            break;
                        default:
                            break;
                    }
                    lux = veml.getLux();
                    if((250*A_value*A_value)/(SP_value_for_calc*lux) < 10000){
                        ISO_value = (250*A_value*A_value)/(SP_value_for_calc*lux);
                        ISO_value_for_calc = (250*A_value*A_value)/(SP_value_for_calc);
                    }
                    if(Last_ISO_value_for_calc != ISO_value_for_calc){
                        Serial.print("ISO_value");Serial.println(ISO_value);
                        if(ISO_value < 10000 && ISO_value >= 0){
                        lcd.setCursor(0, 1);
                        lcd.print("    ");
                        lcd.setCursor(0, 1);
                        lcd.print(ISO_value, 0);
                    }
                    }
                    Last_ISO = ISO_button_state;
                    Last_SP = SP_button_state;
                    Last_A = A_button_state;
                    Last_ISO_value_for_calc = ISO_value_for_calc;
                }
                break;
            case 2:
                while(change_value != 2){
                    ISO_button_state = digitalRead(ISO_button);
                    SP_button_state = digitalRead(SP_button);
                    A_button_state = digitalRead(A_button);
                    if(Last_ISO != ISO_button_state){
                        if(ISO_button_state == HIGH){
                            change_value = 1;
                        }
                    }   
                    if(Last_SP != SP_button_state){
                        if(SP_button_state == HIGH){
                            change_value = 2;
                        }
                    }
                    if(Last_A != A_button_state){
                        if(A_button_state == HIGH){
                            change_value = 3;
                        }
                    }
                    switch(change_value){
                        case 1:
                            plus_button_state = digitalRead(plus_button);
                            minus_button_state = digitalRead(minus_button);
                            if(Last_plus_button_state != plus_button_state){
                            if(plus_button_state == HIGH){
                                ISO_CHANGE_double();
                            }
                        }
                            if(Last_minus_button_state != minus_button_state){
                                if(minus_button_state == HIGH){
                                ISO_CHANGE_decrease();
                            }
                        }
                            Last_plus_button_state = plus_button_state;
                            Last_minus_button_state = minus_button_state;
                            break;
                        case 2:
                            choose_const = -1;
                            go_back_to_normal = 1;
                            break;
                        case 3:
                            plus_button_state = digitalRead(plus_button);
                            minus_button_state = digitalRead(minus_button);
                            if(Last_plus_button_state != plus_button_state){
                            if(plus_button_state == HIGH){
                                if(Apreture_counter + 1 < 29){
                                    Apreture_counter++;
                                    A_value = arr_Apreture[Apreture_counter];
                                    lcd.setCursor(12, 1);
                                    lcd.print(A_value, 2);
                                }
                            }
                        }
                            if(Last_minus_button_state != minus_button_state){
                                if(minus_button_state == HIGH){
                                    if(Apreture_counter - 1 > -1){
                                        Apreture_counter--;
                                        A_value = arr_Apreture[Apreture_counter];
                                        lcd.setCursor(12, 1);
                                        lcd.print(A_value, 2);
                                    }
                                }
                            }
                            Last_plus_button_state = plus_button_state;
                            Last_minus_button_state = minus_button_state;
                            break;
                        default:
                            break;
                    }
                    Last_ISO = digitalRead(ISO_button);
                    Last_SP = digitalRead(SP_button);
                    Last_A = digitalRead(A_button);
                    lux = veml.getLux();
                    SP_value = (250*A_value*A_value)/(ISO_value*lux);
                    SP_value_for_calc = (250*A_value*A_value)/(ISO_value);
                    Serial.print("LUX");Serial.println(lux);
                    Serial.print("SP_value:");Serial.println(SP_value);
                    if(Last_SP_value_for_calc != SP_value_for_calc){
                        if(SP_value < 1){
                            SP_value = 1/SP_value;
                            lcd.setCursor(4, 1);
                            lcd.print("        ");
                            lcd.setCursor(4, 1);
                            lcd.print("1/");
                            lcd.setCursor(6, 1);
                            lcd.print(SP_value, 1);
                        }
                        else{
                            lcd.setCursor(4, 1);
                            lcd.print("        ");
                            lcd.setCursor(5, 1);
                            lcd.print(SP_value, 1);
                        }
                    }
                    Last_SP_value_for_calc = SP_value_for_calc;
                }
                break;
            case 3:
                while(change_value != 3){
                    ISO_button_state = digitalRead(ISO_button);
                    SP_button_state = digitalRead(SP_button);
                    A_button_state = digitalRead(A_button);
                    if(Last_ISO != ISO_button_state){
                        if(ISO_button_state == HIGH){
                            change_value = 1;
                        }
                    }   
                    if(Last_SP != SP_button_state){
                        if(SP_button_state == HIGH){
                            change_value = 2;
                        }
                    }
                    if(Last_A != A_button_state){
                        if(A_button_state == HIGH){
                            change_value = 3;
                        }
                    }
                    switch(change_value){
                        case 1:
                            plus_button_state = digitalRead(plus_button);
                            minus_button_state = digitalRead(minus_button);
                            if(Last_plus_button_state != plus_button_state){
                                if(plus_button_state == HIGH){ 
                                    ISO_CHANGE_double();
                                }
                            }
                            if(Last_minus_button_state != minus_button_state){
                                if(minus_button_state == HIGH){
                                    ISO_CHANGE_decrease();
                                }
                            }
                            Last_plus_button_state = plus_button_state;
                            Last_minus_button_state = minus_button_state;
                            break;
                        case 2:
                            plus_button_state = digitalRead(plus_button);
                            minus_button_state = digitalRead(minus_button);
                            if(Last_plus_button_state != plus_button_state){
                            if(plus_button_state == HIGH){
                                    SP_counter = SP_counter + 1;
                                    SP_CHANGE();
                            }
                        }
                            if(Last_minus_button_state != minus_button_state){
                                if(minus_button_state == HIGH){
                                    SP_counter = SP_counter - 1;
                                    SP_CHANGE();
                            }
                        }
                            Last_plus_button_state = plus_button_state;
                            Last_minus_button_state = minus_button_state;
                            break;
                        case 3:
                            choose_const = -1;
                            go_back_to_normal = 1;
                            break;
                        default:
                            break;
                    }
                    Last_ISO = digitalRead(ISO_button);
                    Last_SP = digitalRead(SP_button);
                    Last_A = digitalRead(A_button);
                    lux = veml.getLux();
                    Serial.print("LUX:");Serial.println(lux);
                    A_value = sqrt((lux*ISO_value*SP_value_for_calc)/250);
                    A_value_for_calc = sqrt((ISO_value*SP_value_for_calc)/250);
                    Serial.print("Aperature:");Serial.println(A_value);
                    if(Last_A_value_for_calc != A_value_for_calc){
                        lcd.setCursor(12, 1);
                        lcd.print("    ");
                        lcd.setCursor(12, 1);
                        lcd.print(A_value);
                    }
                    Last_A_value_for_calc = A_value_for_calc;
                }
                break;
            default: 
                break;
        }
    }
