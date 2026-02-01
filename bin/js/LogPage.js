
import { apiRequest } from "./apiRequest.js";

window.sendLogin = async function sendLogin() {
    const headerValue = { "Content-Type": "application/json", 'alg': 'HS256' };
    const loginValue = document.getElementById("login").value.trim();
    const passwordValue = document.getElementById("password").value;
    //const adminValue = (loginValue == "admin") ? true : false;
    //const bearerValue = JSON.parse(Uint8Array.encode(JSON.stringify(headerValue)));
 
    if (!loginValue || !passwordValue) {
        showMessage("Введите логин и пароль");
        return;
    }

    try {
        const text = await apiRequest("https://localhost:8080/login", {
            method: "POST",
            headers: { "Content-Type": "application/json", 'alg': 'HS256' },
            body: JSON.stringify({ login: loginValue, password: passwordValue/*, adminValue: adminValue, bearer: bearerValue*/}),
            expect: "text"
        });

        console.log("[Login] response", text);

        if (text === "USER_USER_DOESNT_EXIST") {
            showMessage("Пользователь не найден");
        } else if (text === "USER_WRONG_INCORRECT_DATA") {
            showMessage("Неверный логин или пароль");
        } else if (text === "INTERNAL_ERROR") {
            showMessage("Внутренняя ошибка сервера");
        } else {
            showMessage("Вход выполнен. Перенаправление...", "success");
            setTimeout(() => {
                window.location = "/MainPage.html";
            }, 800);
        }

    } catch (err) {
        console.error("[Login] network error", err);
        showMessage("Ошибка сети. Проверьте соединение");
    }
}

function showMessage(text, type = "error") {
    const el = document.getElementById("formMessage");
    el.textContent = text;
    el.className = "form-message " + type;
}
