
document.addEventListener("DOMContentLoaded", () => {

    const addButton = document.getElementById("addButton");
    const fileInput = document.getElementById("fileInput");
    const refreshButton = document.getElementById("refreshButton");

    addButton.addEventListener("click", () => {
        fileInput.click();
    });

    fileInput.addEventListener("change", async () => {
        const files = fileInput.files;
        if (files.length === 0) return;

        const formData = new FormData();

        for (const file of files) {
            const hash = await calculateSHA256(file);

            const metaInfo = {
                name: file.name,
                size: file.size,
                extension: file.name.split('.').pop(),
                type: file.type,
                lastModified: file.lastModified,
                hash: hash
            };

            // файл
            formData.append("files", file);

            // meta для КАЖДОГО файла (ключ одинаковый)
            formData.append(
                "meta",
                JSON.stringify(metaInfo)
            );
        }

        fetch("/upload", {
            method: "POST",
            body: formData
        })
        .then(res => res.text())
        .then(text => {
            console.log("Server:", text);
            refreshFileList();
        })
        .catch(console.error);
    });

    refreshButton.addEventListener("click", refreshFileList);

    // 🔥 при первой загрузке страницы
    refreshFileList();
});

/* ===== utils ===== */

async function calculateSHA256(file) {
    const buffer = await file.arrayBuffer();
    const hashBuffer = await crypto.subtle.digest("SHA-256", buffer);
    const hashArray = Array.from(new Uint8Array(hashBuffer));
    return hashArray.map(b => b.toString(16).padStart(2, "0")).join("");
}

function refreshFileList() {
    fetch("/list")
        .then(res => res.json())
        .then(files => {
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

