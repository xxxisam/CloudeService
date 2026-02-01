export async function apiRequest(url, {
  method = "GET",
  headers = {},
  body = null,
  expect = "text"
} = {}) {
  try {
    const res = await fetch(url, { method, headers, body });

    let data;
    if (expect === "json") data = await res.json();
    else if (expect === "blob") data = await res.blob();
    else data = await res.text();

    if (!res.ok) {
      const err = new Error(data || "Request failed");
      err.status = res.status;
      throw err;
    }

    return data;
  } catch (err) {
    console.error("[API]", url, err);
    throw err;
  }
}