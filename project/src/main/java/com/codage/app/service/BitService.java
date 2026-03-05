package com.codage.app.service;

import org.springframework.stereotype.Service;
import com.codage.app.util.BaseTransformer;

@Service
public class BitService {
    public String process(String method, int value, Integer param) {
        BaseTransformer transformer = new BaseTransformer(value);
        switch (method) {
            case "shiftLeft":
                transformer.shiftLeft(param != null ? param : 1);
                break;
            case "shiftRight":
                transformer.shiftRight(param != null ? param : 1);
                break;
            case "invertBits":
                transformer.invertBits();
                break;
            case "andBits":
                transformer.andBits(param != null ? param : 0);
                break;
            case "orBits":
                transformer.orBits(param != null ? param : 0);
                break;
            case "toBinary":
                return transformer.toBinary();
            case "fromBinary":
                if (param != null) transformer.fromBinary(Integer.toBinaryString(param));
                break;
            default:
                return "Méthode inconnue";
        }
        return transformer.getValue() + " (binaire : " + transformer.toBinary() + ")";
    }
}
