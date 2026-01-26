async function sendLogin() {
    const loginValue = document.getElementById("login").value.trim();
    const passwordValue = document.getElementById("password").value;

    if (!loginValue || !passwordValue) {
        showMessage("Введите логин и пароль");
        return;
    }

    try {
        const res = await fetch("/login", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                login: loginValue,
                password: passwordValue
            })
        });

        const text = (await res.text()).trim();
        console.log("[Login] response", res.status, text);

        if (res.ok) {
            showMessage("Вход выполнен. Перенаправление...", "success");
            setTimeout(() => {
                window.location = "/MainPage.html";
            }, 800);
            return;
        }

        // обработка кодов сервера
        switch (text) {
            case "USER_USER_DOESNT_EXIST":
                showMessage("Пользователь не найден");
                break;

            case "USER_WRONG_INCORRECT_DATA":
                showMessage("Неверный логин или пароль");
                break;

            case "INTERNAL_ERROR":
                showMessage("Внутренняя ошибка сервера");
                break;

            default:
                showMessage("Ошибка входа");
        }

    } catch (err) {
        console.error("[Login] network error", err);
        showMessage("Ошибка сети. Проверьте соединение");
    }
}

document.getElementById("loginForm").addEventListener("submit", async (e) => {
    e.preventDefault();

    const login = document.getElementById("login").value.trim();
    const password = document.getElementById("password").value;

    console.log("[LoginForm] submit", { login });

    if (!login || !password) {
        showMessage("Введите логин и пароль");
        return;
    }

    try {
        const res = await fetch("/login", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ login, password })
        });

        const text = (await res.text()).trim();
        console.log("[LoginForm] response", res.status, text);

        if (res.ok) {
            showMessage("Вход выполнен. Перенаправление...", "success");
            setTimeout(() => {
                window.location = "/MainPage.html";
            }, 800);
            return;
        }

        switch (text) {
            case "USER_USER_DOESNT_EXIST":
                showMessage("Пользователь не найден");
                break;

            case "USER_WRONG_INCORRECT_DATA":
                showMessage("Неверный пароль");
                break;

            case "INTERNAL_ERROR":
                showMessage("Внутренняя ошибка сервера");
                break;

            default:
                showMessage("Ошибка входа");
        }

    } catch (err) {
        console.error("[LoginForm] network error", err);
        showMessage("Ошибка сети. Проверьте соединение");
    }
});

function showMessage(text, type = "error") {
    const el = document.getElementById("formMessage");
    el.textContent = text;
    el.className = "form-message " + type;
}