document.getElementById("form").addEventListener("submit", async (e) => {
    e.preventDefault();
  
    const formData = new FormData(e.target);
    const resultElement = document.getElementById("result");
  
    resultElement.textContent = "⏳ Symulacja w toku...";
  
    try {
      const response = await fetch("/simulate", {
        method: "POST",
        body: formData,
      });
  
      if (!response.ok) throw new Error("Błąd symulacji");
  
      const blob = await response.blob();
      const text = await blob.text();
  
      resultElement.textContent = text;
  
      // Dodaj przycisk do pobrania
      const url = URL.createObjectURL(blob);
      const downloadLink = document.createElement("a");
      downloadLink.href = url;
      downloadLink.download = "result.json";
      downloadLink.textContent = "⬇️ Pobierz wynik";
      document.body.appendChild(downloadLink);
    } catch (err) {
      resultElement.textContent = "❌ Symulacja nie powiodła się: " + err.message;
    }
  });
  