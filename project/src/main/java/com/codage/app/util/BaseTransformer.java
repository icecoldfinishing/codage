package com.codage.app.util;

/**
 * Utilitaire pour la transformation de base et manipulation de bits.
 */
public class BaseTransformer {
    /**
     * Décale la valeur vers la gauche de n bits.
     */
    public void shiftLeft(int n) {
        this.value = this.value << n;
    }
    /**
     * Décale la valeur vers la droite de n bits.
     */
    public void shiftRight(int n) {
        this.value = this.value >> n;
    }
    /**
     * Effectue un ET binaire avec une autre valeur.
     */
    public void andBits(int other) {
        this.value = this.value & other;
    }
    /**
     * Effectue un OU binaire avec une autre valeur.
     */
    public void orBits(int other) {
        this.value = this.value | other;
    }
    
    private int value;

    public BaseTransformer(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    public void setValue(int value) {
        this.value = value;
    }

    public String toBinary() {
        return Integer.toBinaryString(value);
    }

    public void fromBinary(String binary) {
        this.value = Integer.parseInt(binary, 2);
    }

    public void invertBits() {
        this.value = ~this.value;
    }
}
