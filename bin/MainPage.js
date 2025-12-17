document.addEventListener("DOMContentLoaded", function () {
    const addButton = document.getElementById("addButton");
    const fileInput = document.getElementById("fileInput");
    const refreshButton = document.getElementById("refreshButton");

    addButton.addEventListener("click", () => fileInput.click());

    fileInput.addEventListener("change", async () => {
        const file = fileInput.files[0]; // берём первый файл
        if (!file) return;

        // 1️⃣ Вычисляем SHA-256
        const buffer = await file.arrayBuffer();
        const hashBuffer = await crypto.subtle.digest("SHA-256", buffer);
        const hashArray = Array.from(new Uint8Array(hashBuffer));
        const hashHex = hashArray.map(b => b.toString(16).padStart(2, "0")).join("");

        // 2️⃣ Формируем метаинформацию
        const meta = {
            name: file.name,
            extension: file.name.split(".").pop(),
            fullName: file.name,
            size: file.size,
            hash: hashHex
        };

        // 3️⃣ Отправляем метаинформацию на сервер
        const metaRes = await fetch("/upload/meta", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(meta)
        });

        if (!metaRes.ok) {
            console.error("Failed to upload meta info for", file.name);
            return;
        }

        console.log("Meta info uploaded:", file.name);

        // 4️⃣ После успешного приёма метаинформации отправляем файл
        const formData = new FormData();
        formData.append("file", file);
        formData.append("hash", hashHex); // сервер может использовать для привязки к метаинфе

        const fileRes = await fetch("/upload", {
            method: "POST",
            body: formData
        });

        if (!fileRes.ok) {
            console.error("Failed to upload file", file.name);
        } else {
            console.log("File uploaded:", file.name);
        }

        // 5️⃣ Обновляем список файлов
        refreshFileList();
    });

    refreshButton.addEventListener("click", refreshFileList);

    // при первой загрузке страницы
    refreshFileList();
});



function refreshFileList() {
  fetch("/list")
    .then((res) => res.json())
    .then((files) => {
      const tbody = document.getElementById("fileTableBody");
      tbody.innerHTML = "";

      for (const file of files) {
        const tr = document.createElement("tr");
        tr.innerHTML = `
                    <td>${file.name}</td>
                    <td>${file.hash ?? "-"}</td>
                    <td>${file.size}</td>
                    <td>${file.extension}</td>
                `;
        tbody.appendChild(tr);
      }
    })
    .catch(console.error);
}
