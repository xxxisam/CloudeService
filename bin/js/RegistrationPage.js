document.getElementById("registrationForm").addEventListener("submit", async (e) => {
    e.preventDefault();

    const login = document.getElementById("login").value.trim();
    const password = document.getElementById("password").value;
    const password_repeat = document.getElementById("password_repeat").value;

    console.log("[RegisterForm] submit", { login });

    if (password !== password_repeat) {
        console.warn("[RegisterForm] passwords do not match");
        showMessage("Пароли не совпадают");
        return;
    }

    try {
        const res = await fetch("https://localhost:8080/registration", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ login, password, password_repeat })
        });

        const text = await res.text();
        console.log("[RegisterForm] response", res.status, text);

        if (res.ok) {
            showMessage("Регистрация успешна! Перенаправление...", "success");
            setTimeout(() => {
                window.location = "/LogPage.html";
            }, 800);
            return;
        }

        
        switch (res.status) {
            case 409:
                showMessage("Пользователь с таким логином уже существует");
                break;
            case 400:
                showMessage("Некорректные данные формы");
                break;
            default:
                showMessage("Ошибка сервера. Попробуйте позже");
        }

    } catch (err) {
        console.error("[RegisterForm] network error", err);
        showMessage("Ошибка сети. Проверьте соединение");
    }
});

function showMessage(text, type = "error") {
    const el = document.getElementById("formMessage");
    el.textContent = text;
    el.className = "form-message " + type;
}