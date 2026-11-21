#pragma once
// Copiar a CloudSecrets.h y completar localmente; no publicar el archivo privado.
namespace CloudSecrets {
constexpr char WIFI_SSID[] = "";
constexpr char WIFI_PASSWORD[] = "";
constexpr char AWS_ENDPOINT[] = ""; // endpoint ATS, sin https:// ni puerto
constexpr char THING_NAME[] = "";
constexpr char ROOT_CA[] = R"PEM(
-----BEGIN CERTIFICATE-----
PEGAR_CERTIFICADO_RAIZ_AMAZON
-----END CERTIFICATE-----
)PEM";
constexpr char DEVICE_CERT[] = R"PEM(
-----BEGIN CERTIFICATE-----
PEGAR_CERTIFICADO_DEL_DISPOSITIVO
-----END CERTIFICATE-----
)PEM";
constexpr char PRIVATE_KEY[] = R"PEM(
-----BEGIN PRIVATE KEY-----
PEGAR_CLAVE_PRIVADA
-----END PRIVATE KEY-----
)PEM";
}
