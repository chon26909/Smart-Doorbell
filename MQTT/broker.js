const mosca = require("mosca");

const username = "chon";
const password = "1234";
const PORT = process.env.PORT || 1883;
const settings = { port: PORT };

const broker = new mosca.Server(settings);

broker.on("ready", () => {
  console.log("Server is ready!!");
  broker.authenticate = authenticate;
});

const authenticate = (client, _username, _password, callback) => {
  const authorized = _username == username && _password.toString() == password;
  if (authorized) client.user == username;
  callback(null, authorized);
};

broker.on("published", (packet) => {
  console.log(packet);
});
