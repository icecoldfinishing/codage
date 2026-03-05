package com.codage.app.util;

/**
 * Utilitaire pour manipuler des bits et faire du codage binaire.
 */
public class BitUtils {
    /**
     * Retourne la valeur du bit à la position donnée dans un entier.
     */
    public static int getBit(int value, int position) {
        return (value >> position) & 1;
    }

    /**
     * Met à 1 le bit à la position donnée dans un entier.
     */
    public static int setBit(int value, int position) {
        return value | (1 << position);
    }

    /**
     * Met à 0 le bit à la position donnée dans un entier.
     */
    public static int clearBit(int value, int position) {
        return value & ~(1 << position);
    }

    /**
     * Inverse le bit à la position donnée dans un entier.
     */
    public static int toggleBit(int value, int position) {
        return value ^ (1 << position);
    }
}
