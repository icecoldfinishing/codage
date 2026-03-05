package com.codage.app;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class CodageApplication {

   public static void main(String[] args) {
       // Exemple orienté objet avec BaseTransformer
       com.codage.app.util.BaseTransformer transformer = new com.codage.app.util.BaseTransformer(13); // 1101
       System.out.println("Valeur initiale : " + transformer.getValue());
       System.out.println("Binaire : " + transformer.toBinary());

       transformer.invertBits();
       System.out.println("Après inversion des bits : " + transformer.getValue() + " (binaire : " + transformer.toBinary() + ")");

       transformer.fromBinary("1010");
       System.out.println("Après fromBinary('1010') : " + transformer.getValue() + " (binaire : " + transformer.toBinary() + ")");
   }
}
