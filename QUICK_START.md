# ⚡ Quick Start - Rodar Servidor WYD

## 🚀 Resumo Rápido (5 Minutos)

### 1️⃣ MySQL
```bash
# Importar banco
mysql -u root -p < Server/banco.sql

# Verificar
mysql -u root -p
USE wydsite;
SHOW TABLES;
```

### 2️⃣ Configurar IPs

**Para testes locais (mesma máquina)**:

```
# Server/DBSrv/run/localip.txt
127.0.0.1

# Server/DBSrv/run/serverlist.txt
0  0  127.0.0.1
0  1  127.0.0.1
```

### 3️⃣ Iniciar Servidores

```bash
# Janela 1 - DBSrv
cd Server/DBSrv/run
./DBSrv.exe

# Janela 2 - TMSrv (aguardar DBSrv iniciar primeiro!)
cd Server/TMSrv/run
./TMSrv.exe
```

### 4️⃣ Configurar Cliente

```
# Usar: Server/Configurar o IP.exe
Copiar para Cliente/
Executar
Digitar: 127.0.0.1
```

### 5️⃣ Conectar

```
Executar: Cliente/Resolution The New World.exe
Login: 1111
Senha: 1111
Numeric: 001010
```

---

## 🎮 Testar RedDragon Items

```
# No jogo (chat):
/gmmode
/level 400
/gold 500000000
/spawn 5762 1    # Garra RedDragon
/spawn 5753 30   # Escamas
/summon 9500     # Boss Ignis
```

---

## ❌ Erros Comuns

### "Can't connect to MySQL"
```bash
# Verificar se MySQL está rodando
services.msc → MySQL80 → Start
```

### "Items não aparecem"
```bash
# Deletar e recriar ItemList.bin
cd Server/TMSrv/run
rm ItemList.bin
# Reiniciar TMSrv
```

### "Failed to connect to server"
```bash
# Verificar ordem:
1. MySQL rodando? ✅
2. DBSrv rodando? ✅
3. TMSrv rodando? ✅
4. Config.bin com IP correto? ✅
```

---

## 📖 Guia Completo

Ver: **SERVER_SETUP_TUTORIAL.md**

---

**Seu IP**: 21.0.0.140
**Conta Teste**: 1111/1111
**Porta**: 8281
