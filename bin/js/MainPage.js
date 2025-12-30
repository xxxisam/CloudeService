async function uploadFile(file) {
  const hashHex = await computeFileHash(file);

  const chunkSize = 1 * 1024 * 1024;
  //const chunkSize = 4096;
  const packetCount = Math.ceil(file.size / chunkSize);
  console.log(packetCount);
  // let packetCount = file.size % 4096;
  // let remainingPackets = file.size;
  for (let id = 0; id < packetCount; ++id) {
    const start = id * chunkSize;
    const end = Math.min(start + chunkSize, file.size);
    const chunk = file.slice(start, end);

    const fullName = file.name;
    const lastDot = fullName.lastIndexOf(".");
    const name = lastDot === -1 ? fullName : fullName.substring(0, lastDot);
    const extension = lastDot === -1 ? "" : fullName.substring(lastDot);

    const meta = {
      name: name,
      fullName: fullName,
      extension: extension,
      size: file.size,
      hash: hashHex,
      packetCount: packetCount,
      packetID: id,
    };

    // 2. multipart/form-data
    const formData = new FormData();
    formData.append("meta", JSON.stringify(meta));
    formData.append("file", chunk); // бинарные данные

    // 3. Один запрос
    const res = await fetch("/upload", {
      method: "POST",
      body: formData,
    });

    if (!res.ok) {
      const text = await res.text();
      throw new Error(text);
    }

    console.log("Upload OK", id);
    // if (remainingPackets - 4096 > 4096) {
    //   remainingPackets -= 4096;
    // } else if (remainingPackets - 4096 > 0) {
    //   remainingPackets -= 4096;
    // } else {
    //   remainPacket;
    // }
  }
}

document.addEventListener("DOMContentLoaded", () => {
  const addButton = document.getElementById("addButton");
  const refreshButton = document.getElementById("refreshButton");
  const fileInput = document.getElementById("fileInput");

  addButton.addEventListener("click", () => fileInput.click());

  refreshButton.addEventListener("click", () => {
    console.log("Refresh button clicked");
    refreshFileList();
  });

  fileInput.addEventListener("change", async () => {
    const file = fileInput.files[0];
    if (!file) return;

    try {
      await uploadFile(file);
      refreshFileList();
    } catch (e) {
      console.error(e);
      alert("Ошибка загрузки");
    }
  });
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

                const actionTd = document.createElement("td");

                // Кнопка удаления
                const deleteBtn = createDeleteButton(file);
                actionTd.appendChild(deleteBtn);

                // Кнопка скачивания
                const downloadBtn = createDownloadButton(file);
                actionTd.appendChild(downloadBtn);

                tr.appendChild(actionTd);
                tbody.appendChild(tr);
            }
        })
        .catch(console.error);
}

// Создание кнопки download с обработчиком
function createDownloadButton(file) {
    const btn = document.createElement("button");
    btn.textContent = "Download";
    btn.addEventListener("click", () => handleDownload(file));
    return btn;
}

// Функция обработки скачивания файла
async function handleDownload(file) {
  const CHUNK_SIZE = 1 * 1024 * 1024; // 1 MB
  let receivedBytes = 0;
  const chunks = [];

  try {
    while (receivedBytes < file.size) {
      const end = Math.min(receivedBytes + CHUNK_SIZE, file.size);

      const res = await fetch(`/download?hash=${file.hash}`, {
        headers: { 'Range': `bytes=${receivedBytes}-${end - 1}` }
      });

      if (!res.ok) throw new Error(`Failed to download chunk at byte ${receivedBytes}`);

      const chunk = new Uint8Array(await res.arrayBuffer());
      chunks.push(chunk);

      receivedBytes += chunk.length;

      console.log(`Downloaded ${receivedBytes} / ${file.size} bytes`);
    }

    // Собираем все чанки в Blob
    const blob = new Blob(chunks, { type: "application/octet-stream" });

    // Создаём ссылку для скачивания
    const link = document.createElement("a");
    link.href = URL.createObjectURL(blob);
    link.download = file.fullName;
    link.click();

    // Освобождаем объект URL
    URL.revokeObjectURL(link.href);

    console.log("Download complete:", file.name);
  } catch (err) {
    console.error("Download failed:", err);
    alert("Ошибка скачивания файла");
  }
}

document.addEventListener("DOMContentLoaded", () => {
  refreshFileList();
});

async function computeFileHash(file) {
  const buffer = await file.arrayBuffer();
  const hashBuffer = await crypto.subtle.digest("SHA-256", buffer);
  const hashHex = Array.from(new Uint8Array(hashBuffer))
    .map((b) => b.toString(16).padStart(2, "0"))
    .join("");
  return hashHex;
}

function createDeleteButton(file) {
    const btn = document.createElement("button");
    btn.textContent = "Delete";

    btn.addEventListener("click", async () => {
        try {
            const res = await fetch(`/delete?hash=${file.hash}`, { method: "DELETE" });
            if (res.ok) {
                refreshFileList(); // централизованный вызов обновления
            } else {
                console.error("Ошибка удаления");
            }
        } catch (err) {
            console.error(err);
        }
    });

    return btn;
}
