package com.codage.app.controller;

import com.codage.app.service.BitService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;

@Controller
public class BitController {
    @Autowired
    private BitService bitService;

    @GetMapping("/")
    public String showForm() {
        return "views/bitform";
    }

    @PostMapping("/result")
    public String processForm(@RequestParam String method,
                              @RequestParam int value,
                              @RequestParam(required = false) Integer param,
                              Model model) {
        String result = bitService.process(method, value, param);
        model.addAttribute("result", result);
        return "views/bitform";
    }
}
