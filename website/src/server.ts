import express from "express";
import path from "path";

const app = express();
const PORT = 4000;

app.use(express.static(path.join(process.cwd(), "public")));
app.get(/^\/.*$/, (req, res) => {
  res.sendFile(path.join(process.cwd(), "public", "index.html"));
});
app.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}`);
});
