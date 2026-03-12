#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 29, 177);
IPAddress upstreamDNS(8, 8, 8, 8);  // Google DNS

EthernetUDP udpIn;   // For incoming DNS requests
EthernetUDP udpOut;  // For forwarding to upstream

#define DNS_PORT 53
#define BUFFER_SIZE 512

byte packetBuffer[BUFFER_SIZE];

// Basic ad/tracker keywords for blocking
const char* blockedKeywords[] = {
  "doubleclick.net",
  "googlesyndication.com",
  "googleadservices.com",
  "googletagservices.com",
  "pagead2.googlesyndication.com",
  "pagead.l.doubleclick.net",
  "pubads.g.doubleclick.net",
  "adservice.google.com",
  "adservice.google.co.in",
  "www-google-analytics.l.google.com",
  "static.doubleclick.net",
  "tpc.googlesyndication.com",
  "googleads.g.doubleclick.net",
  "tracking-protection.cdn.mozilla.net",
  "metrics.icloud.com",
  "adserver.adtechus.com",
  "ads.twitter.com",
  "ads.facebook.com",
  "connect.facebook.net",
  "adnxs.com",
  "taboola.com",
  "outbrain.com",
  "zedo.com",
  "yieldmanager.com",
  "scorecardresearch.com",
  "moatads.com",
  "adform.net",
  "smartadserver.com",
  "rubiconproject.com",
  "serving-sys.com",
  "krxd.net",
  "quantserve.com",
  "bluekai.com",
  "demdex.net",
  "rfihub.com",
  "simpli.fi",
  "openx.net",
  "contextweb.com",
  "bidswitch.net",
  "exelator.com",
  "everesttech.net"
};
const int blockedCount = sizeof(blockedKeywords) / sizeof(blockedKeywords[0]);

// Helper function to convert a string to lowercase
String toLowercase(const String& str) {
  String lowerStr = str;
  lowerStr.toLowerCase();
  return lowerStr;
}

// Check if the domain matches any blocked keyword
bool matchesBlockedKeyword(const String& domain) {
  for (int i = 0; i < blockedCount; i++) {
    if (domain.indexOf(blockedKeywords[i]) >= 0) {
      return true;
    }
  }
  return false;
}

// Check if domain is a YouTube video content server (allowed)
bool isYouTubeVideoHost(const String& domain) {
  // Matches r*.googlevideo.com or rr*.googlevideo.com
  if (domain.endsWith(".googlevideo.com")) {
    int len = domain.length();
    int pos = domain.indexOf(".googlevideo.com");
    if (pos > 0) {
      String prefix = domain.substring(0, pos);
      // Check if prefix matches r or rr followed by digits or dashes (e.g. r1, rr2)
      if (prefix.startsWith("r") || prefix.startsWith("rr")) {
        // Allow these hosts as they serve video content
        return true;
      }
    }
  }
  return false;
}

// Check if domain is a suspicious YouTube domain to block
bool isSuspiciousYouTubeDomain(const String& domain) {
  // Block known ad/tracker subdomains and domains containing 'ad' or 'manifest' in suspicious contexts
  // Common blocked YouTube ad/tracker domains
  const char* youtubeBlocked[] = {
    "ads.youtube.com",
    "ad.youtube.com",
    "s.youtube.com",
    "pagead2.googlesyndication.com",
    "youtubei.googleapis.com",
    "rr1---sn",
    "rr2---sn",
    "rr3---sn",
    "rr4---sn",
    "rr5---sn",
    "rr6---sn",
    "rr7---sn",
    "rr8---sn",
    "rr9---sn",
    "rr10---sn"
  };
  const int ytBlockedCount = sizeof(youtubeBlocked) / sizeof(youtubeBlocked[0]);

  for (int i = 0; i < ytBlockedCount; i++) {
    if (domain.indexOf(youtubeBlocked[i]) >= 0) {
      return true;
    }
  }

  // Additionally block domains containing 'ad' or 'manifest' in their names (case-insensitive)
  if (domain.indexOf("ad") >= 0 || domain.indexOf("manifest") >= 0) {
    return true;
  }

  return false;
}

// Refined and efficient domain blocking function
bool isBlockedDomain(const String& domain) {
  String lowerDomain = toLowercase(domain);

  // Allow YouTube video content hosts explicitly
  if (isYouTubeVideoHost(lowerDomain)) {
    return false;
  }

  // Block suspicious YouTube domains
  if (isSuspiciousYouTubeDomain(lowerDomain)) {
    return true;
  }

  // Block general ad/tracker domains
  if (matchesBlockedKeyword(lowerDomain)) {
    return true;
  }

  return false;
}

String parseDNSQueryDomain(byte* packet, int length) {
  int i = 12;  // DNS header = 12 bytes
  String domain = "";
  while (i < length) {
    byte len = packet[i++];
    if (len == 0) break;
    for (int j = 0; j < len; j++) {
      if (i < length)
        domain += (char)packet[i++];
    }
    if (packet[i] != 0 && i < length) domain += ".";
  }
  return domain;
}

void sendDNSReply(IPAddress clientIP, unsigned int clientPort, byte* query, int queryLength, IPAddress answerIP) {
  udpIn.beginPacket(clientIP, clientPort);

  // Header
  udpIn.write(query, 2);  // Transaction ID
  udpIn.write(0x81);
  udpIn.write(0x80);  // Flags: standard response
  udpIn.write((uint8_t)0x00);
  udpIn.write((uint8_t)0x01);  // Questions = 1
  udpIn.write((uint8_t)0x00);
  udpIn.write((uint8_t)0x01);  // Answers = 1
  udpIn.write((uint8_t)0x00);
  udpIn.write((uint8_t)0x00);  // Authority = 0
  udpIn.write((uint8_t)0x00);
  udpIn.write((uint8_t)0x00);  // Additional = 0

  // Question section
  for (int i = 12; i < queryLength; i++) {
    udpIn.write(query[i]);
    if (query[i] == 0 && i + 4 < queryLength) {
      udpIn.write(query[i + 1]);
      udpIn.write(query[i + 2]);
      udpIn.write(query[i + 3]);
      udpIn.write(query[i + 4]);
      break;
    }
  }

  // Answer section
  udpIn.write(0xC0);
  udpIn.write(0x0C);  // pointer to domain name
  udpIn.write((uint8_t)0x00);
  udpIn.write((uint8_t)0x01);  // type A
  udpIn.write((uint8_t)0x00);
  udpIn.write((uint8_t)0x01);  // class IN
  udpIn.write((uint8_t)0x00);
  udpIn.write((uint8_t)0x00);
  udpIn.write((uint8_t)0x00);
  udpIn.write(0x3C);  // TTL = 60s
  udpIn.write((uint8_t)0x00);
  udpIn.write((uint8_t)0x04);  // data length = 4
  udpIn.write(answerIP[0]);
  udpIn.write(answerIP[1]);
  udpIn.write(answerIP[2]);
  udpIn.write(answerIP[3]);

  udpIn.endPacket();
}

void setup() {
  Ethernet.init(10);
  Ethernet.begin(mac, ip);
  udpIn.begin(DNS_PORT);
  udpOut.begin(53000);  // use an ephemeral port for forwarding

  Serial.begin(9600);
  Serial.print("DNS AdBlock server running on ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  int packetSize = udpIn.parsePacket();
  if (packetSize) {
    IPAddress clientIP = udpIn.remoteIP();
    unsigned int clientPort = udpIn.remotePort();
    udpIn.read(packetBuffer, BUFFER_SIZE);

    String domain = parseDNSQueryDomain(packetBuffer, packetSize);
    Serial.print("Query for: ");
    Serial.println(domain);

    if (isBlockedDomain(domain)) {
      Serial.println(" → Blocked (replying with 0.0.0.0)");
      sendDNSReply(clientIP, clientPort, packetBuffer, packetSize, IPAddress(0, 0, 0, 0));
    } else {
      Serial.println(" → Forwarding upstream...");

      // Forward the query to upstream DNS
      udpOut.beginPacket(upstreamDNS, 53);
      udpOut.write(packetBuffer, packetSize);
      udpOut.endPacket();

      // Wait for response
      unsigned long start = millis();
      bool gotReply = false;
      int len = 0;

      while (millis() - start < 1000) {  // wait up to 1 second
        int respSize = udpOut.parsePacket();
        if (respSize) {
          len = udpOut.read(packetBuffer, BUFFER_SIZE);
          gotReply = true;
          break;
        }
      }

      if (gotReply) {
        // Relay the upstream reply back to the client
        udpIn.beginPacket(clientIP, clientPort);
        udpIn.write(packetBuffer, len);
        udpIn.endPacket();
        Serial.println(" → Forwarded successfully.");
      } else {
        Serial.println(" ⚠️ No upstream reply — ignoring.");
      }
    }
  }
}