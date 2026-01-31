# 🎮 Tutorial Completo - Rodando Servidor WYD Localmente

## 📋 Pré-requisitos

### Software Necessário
- **MySQL Server 5.7+** ou **8.0+**
- **Windows** (os executáveis são .exe para Windows)
- **Cliente WYD** (Resolution The New World.exe está em `/Cliente/`)

### Porta do Sistema
**IP Atual**: `21.0.0.140` (será usado como host do servidor)

---

## 🗄️ PASSO 1: Configurar Banco de Dados MySQL

### 1.1: Instalar MySQL (se ainda não tiver)

**Download**: https://dev.mysql.com/downloads/installer/

**Instalação**:
```
1. Baixar MySQL Installer (mysql-installer-community)
2. Executar instalador
3. Escolher "Developer Default" ou "Server Only"
4. Configurar senha do root (IMPORTANTE: lembrar dessa senha!)
5. Porta padrão: 3306
6. Finalizar instalação
```

---

### 1.2: Criar Banco de Dados

**Opção A: Via MySQL Workbench (Recomendado)**

```
1. Abrir MySQL Workbench
2. Conectar ao localhost (root/senha_que_criou)
3. Ir em: File → Open SQL Script
4. Selecionar: projeto-wyd/Server/banco.sql
5. Executar (ícone de raio ⚡ ou Ctrl+Shift+Enter)
6. Aguardar conclusão (vai criar database 'wydsite' e todas tabelas)
```

**Opção B: Via Linha de Comando**

```bash
# Abrir CMD (Prompt de Comando) como Administrador
cd C:\Program Files\MySQL\MySQL Server 8.0\bin

# Logar no MySQL
mysql -u root -p
# Digitar senha do root

# Importar banco
USE mysql;
source C:\caminho\para\projeto-wyd\Server\banco.sql;

# Verificar se criou
SHOW DATABASES;
# Deve aparecer 'wydsite'

USE wydsite;
SHOW TABLES;
# Deve aparecer: accounts, balanceamento, characteres, etc.
```

---

### 1.3: Verificar Contas de Teste

O banco já vem com contas pré-criadas para testes:

| Username | Password | Donate Points |
|----------|----------|---------------|
| 1111 | 1111 | 10,000 |
| 2222 | 2222 | 1,700 |
| junior | junior | 19,500 |
| beta1 | beta1 | 4,500 |

**Para criar nova conta via SQL**:
```sql
INSERT INTO accounts (user_id, username, password, donate)
VALUES (1, 'SEUNOME', 'SUASENHA', 0);
```

---

## 🔧 PASSO 2: Configurar IPs do Servidor

### 2.1: IP do Sistema

Seu IP atual: **21.0.0.140**

**Para testes locais (mesma máquina)**:
- Use: `127.0.0.1` ou `localhost`

**Para acesso em rede local**:
- Use: `21.0.0.140`

**Para acesso via internet** (se quiser amigos conectarem):
- Use: Seu IP público (verificar em https://meuip.com.br/)
- Configurar port forwarding no roteador (porta 8281)

---

### 2.2: Configurar DBSrv

**Arquivo 1**: `Server/DBSrv/run/localip.txt`

```
# Para testes locais (mesma máquina):
127.0.0.1

# OU para rede local:
21.0.0.140

# OU para internet:
SEU_IP_PUBLICO
```

**Arquivo 2**: `Server/DBSrv/run/serverlist.txt`

```
# Formato: ServerID  Channel  IP
0  0  127.0.0.1
0  1  127.0.0.1

# OU para rede local:
0  0  21.0.0.140
0  1  21.0.0.140
```

**Explicação**:
- **ServerID**: ID do servidor (0 = primeiro)
- **Channel**: Canal do servidor (0, 1, 2...)
- **IP**: IP onde TMSrv está rodando

---

### 2.3: Configurar TMSrv (Não tem arquivo específico de IP)

O TMSrv usa a configuração do DBSrv automaticamente via `serverlist.txt`.

---

### 2.4: Configurar Cliente

**Arquivo**: `Cliente/Config.bin`

Este arquivo é binário, então precisa ser editado com editor hexadecimal OU recriar:

**Opção A: Usar Editor de ServerList**

```
1. Executar: Server/serverlist editor.exe
2. Carregar: Cliente/Config.bin (se existir)
3. Configurar:
   - Server Name: WYD Local
   - Server IP: 127.0.0.1 (ou 21.0.0.140)
   - Port: 8281
4. Salvar: Cliente/Config.bin
```

**Opção B: Usar Ferramenta "Configurar o IP.exe"**

```
1. Copiar Server/Configurar o IP.exe para pasta Cliente/
2. Executar Configurar o IP.exe
3. Digitar IP: 127.0.0.1 (ou 21.0.0.140)
4. Confirmar (vai atualizar Config.bin)
```

---

## 🚀 PASSO 3: Iniciar Servidores

### 3.1: Ordem de Inicialização

**SEMPRE nesta ordem**:
1. MySQL Server (deve estar rodando)
2. DBSrv.exe
3. TMSrv.exe
4. Cliente

---

### 3.2: Iniciar DBSrv

```
1. Abrir CMD (Prompt de Comando)
2. Navegar: cd C:\caminho\projeto-wyd\Server\DBSrv\run
3. Executar: DBSrv.exe
```

**Output Esperado**:
```
[INFO] DBSrv Starting...
[INFO] Connected to MySQL database: wydsite
[INFO] Listening on port 8281
[INFO] Server ready!
```

**Erros Comuns**:

❌ **"Can't connect to MySQL server"**
- Solução: Verificar se MySQL está rodando
- Verificar: services.msc → MySQL80 → Iniciar

❌ **"Access denied for user"**
- Solução: Verificar usuário/senha do MySQL
- Arquivo de config pode estar em `DBSrv/run/` (procurar por database.ini ou similar)

---

### 3.3: Iniciar TMSrv

```
1. Abrir OUTRO CMD (nova janela)
2. Navegar: cd C:\caminho\projeto-wyd\Server\TMSrv\run
3. Executar: TMSrv.exe
```

**Output Esperado**:
```
[INFO] TMSrv Starting...
[INFO] Loading ItemList.csv... OK
[INFO] Loading Mobs... OK
[INFO] Connecting to DBSrv... OK
[INFO] Server Channel 0 ready!
[INFO] Listening on port 8281
```

**Erros Comuns**:

❌ **"Cannot connect to DBSrv"**
- Solução: Verificar se DBSrv está rodando PRIMEIRO
- Verificar IP em serverlist.txt

❌ **"ItemList.csv not found"**
- Solução: Verificar se está executando de dentro de `TMSrv/run/`
- ItemList.csv, Itemname.csv devem estar na mesma pasta

❌ **"Port 8281 already in use"**
- Solução: Fechar TMSrv anterior ou mudar porta

---

### 3.4: Verificar se Servidores Estão Rodando

**No CMD do DBSrv**, deve aparecer:
```
[INFO] TMSrv connected from 127.0.0.1
[INFO] Channel 0 registered
```

**No CMD do TMSrv**, deve aparecer:
```
[INFO] Connected to DBSrv successfully
[INFO] Waiting for players...
```

---

## 🎮 PASSO 4: Conectar com Cliente

### 4.1: Executar Cliente

```
1. Navegar: projeto-wyd/Cliente/
2. Executar: Resolution The New World.exe
```

---

### 4.2: Fazer Login

**Tela de Login**:
```
Username: 1111
Password: 1111
Numeric: 001010
```

**OU criar nova conta**:
```sql
-- No MySQL Workbench:
INSERT INTO wydsite.accounts (user_id, username, password, numerica)
VALUES (1, 'teste', 'teste', 123456);
```

---

### 4.3: Criar Personagem

```
1. Após login, clicar em "Criar Personagem"
2. Escolher nome
3. Escolher classe (TransKnight, Foema, BeastMaster, Huntress)
4. Confirmar
5. Entrar no jogo
```

---

## 🔨 PASSO 5: Testar Sistema RedDragon

### 5.1: Comandos GM (Game Master)

**Ativar GM Mode** (se sua conta tiver privilégios):

No jogo, abrir chat (Enter) e digitar:

```
/gmmode           # Ativar modo GM
/level 400        # Setar level para 400
/gold 500000000   # Adicionar 500M gold
```

---

### 5.2: Spawnar Items RedDragon

```
/spawn 5752 10    # 10x Alma do Dragão
/spawn 5753 30    # 30x Escama RedDragon
/spawn 5754 20    # 20x Chama Infernal
/spawn 5755 5     # 5x Coração de Dragão
/spawn 5757 5     # 5x Essência do Fogo
/spawn 5758 5     # 5x Essência do Gelo
/spawn 5759 5     # 5x Essência do Trovão
/spawn 5760 5     # 5x Essência da Terra
/spawn 5761 5     # 5x Essência do Vento
```

**Items de Equipamento**:
```
/spawn 5762 1     # Garra RedDragon
/spawn 5763 1     # Cajado RedDragon
/spawn 5764 1     # Elmo RedDragon
/spawn 5765 1     # Armadura RedDragon
/spawn 5766 1     # Calça RedDragon
/spawn 5767 1     # Manoplas RedDragon
/spawn 5768 1     # Botas RedDragon
/spawn 5769 1     # Colar RedDragon
/spawn 5770 1     # Brinco RedDragon
/spawn 5771 1     # Anel RedDragon
```

---

### 5.3: Spawnar Boss RedDragon Ignis

```
/summon 9500      # Spawnar RedDragon Ignis
/killall          # Matar todos mobs (se quiser testar drops)
```

**Testar Drops**:
```
1. Matar o boss (ou usar /killall)
2. Verificar se dropa:
   - Escama RedDragon (100%)
   - Alma do Dragão (80%)
   - Chama Infernal (60%)
   - Essências (30% cada)
```

---

### 5.4: Testar Dragon Infusion (NPC)

**IMPORTANTE**: O NPC ainda não está implementado no jogo!

Você precisará:
1. Adicionar NPC ID 9600 ao banco de dados
2. Posicionar em Armia (coordenadas ~2100, 1900)
3. Configurar para abrir janela de combine tipo "DRAGONFORGE"

**OU** testar via código diretamente (se compilar o servidor).

---

## 🐛 Troubleshooting - Problemas Comuns

### Problema 1: "Failed to connect to server"

**Causas possíveis**:
- Servidores não estão rodando
- IP configurado errado
- Firewall bloqueando porta 8281

**Soluções**:
```
1. Verificar se DBSrv e TMSrv estão rodando
2. Verificar logs em Server/DBSrv/run/output.txt
3. Verificar Config.bin do cliente (deve ter IP correto)
4. Desabilitar firewall temporariamente para testar
5. Adicionar exceção para porta 8281:
   - Windows Firewall → Configurações Avançadas
   - Regras de Entrada → Nova Regra
   - Porta TCP 8281 → Permitir
```

---

### Problema 2: "Account not found" ou "Wrong password"

**Soluções**:
```sql
-- Verificar se conta existe:
SELECT * FROM wydsite.accounts WHERE username = '1111';

-- Se não existir, criar:
INSERT INTO wydsite.accounts (user_id, username, password, numerica)
VALUES (1, '1111', '1111', 001010);

-- Verificar senha:
UPDATE wydsite.accounts SET password = '1111' WHERE username = '1111';
```

---

### Problema 3: Items Não Aparecem (IDs 5752-5771)

**Causas**:
- ItemList.csv não foi carregado
- ItemList.bin desatualizado

**Soluções**:
```
1. Parar TMSrv
2. Deletar: Server/TMSrv/run/ItemList.bin
3. Verificar se ItemList.csv tem linhas 5752-5771
4. Reiniciar TMSrv (vai recriar .bin a partir do .csv)
5. Tentar spawnar items novamente
```

---

### Problema 4: Cliente Fecha/Crasha ao Conectar

**Soluções**:
```
1. Verificar se ClientPatch.dll está na pasta Cliente/
2. Executar cliente como Administrador
3. Desabilitar antivírus temporariamente
4. Verificar logs do cliente (se houver)
5. Reinstalar Visual C++ Redistributables:
   - https://aka.ms/vs/17/release/vc_redist.x86.exe
   - https://aka.ms/vs/17/release/vc_redist.x64.exe
```

---

### Problema 5: "MySQL LIBMYSQL.dll not found"

**Solução**:
```
1. Copiar LIBMYSQL.dll para:
   - Server/DBSrv/run/
   - Server/TMSrv/run/
   - C:\Windows\System32\
2. Ou instalar MySQL Connector C++:
   - https://dev.mysql.com/downloads/connector/cpp/
```

---

## 🌐 PASSO 6: Acesso via Rede/Internet (Opcional)

### Para Amigos na Mesma Rede Local

**1. Configurar IP**:
```
# Em todos arquivos de config, usar:
21.0.0.140
```

**2. Liberar Firewall**:
```
- Windows Firewall → Permitir porta 8281
- Ou desabilitar firewall temporariamente
```

**3. Amigos Conectam**:
```
- Cliente deles: Config.bin com IP 21.0.0.140
- Mesma porta: 8281
```

---

### Para Acesso via Internet

**1. Descobrir IP Público**:
```
Acessar: https://meuip.com.br/
Exemplo: 200.123.45.67
```

**2. Configurar Port Forwarding no Roteador**:
```
1. Acessar roteador (geralmente 192.168.1.1)
2. Login: admin/admin (ou ver no roteador)
3. Procurar: Port Forwarding / Virtual Server / NAT
4. Adicionar regra:
   - Porta Externa: 8281
   - Porta Interna: 8281
   - IP Interno: 21.0.0.140
   - Protocolo: TCP
5. Salvar e reiniciar roteador
```

**3. Testar Port Forward**:
```
Site: https://www.yougetsignal.com/tools/open-ports/
Porta: 8281
Deve aparecer: "Port 8281 is open"
```

**4. Distribuir Cliente**:
```
- Configurar Config.bin com seu IP público
- Enviar pasta Cliente/ inteira para amigos
- Eles executam Resolution The New World.exe
```

---

## 📊 Verificação Final - Checklist

### Banco de Dados ✅
```
[ ] MySQL instalado e rodando
[ ] Database 'wydsite' criada
[ ] Tabelas criadas (accounts, characteres, etc.)
[ ] Conta de teste funciona (1111/1111)
```

### Servidor ✅
```
[ ] DBSrv rodando sem erros
[ ] TMSrv rodando sem erros
[ ] Logs não mostram erros críticos
[ ] serverlist.txt com IP correto
```

### Cliente ✅
```
[ ] Config.bin com IP correto
[ ] ClientPatch.dll presente
[ ] Consegue abrir tela de login
[ ] Consegue logar com 1111/1111
[ ] Consegue criar personagem
[ ] Consegue entrar no jogo
```

### Items RedDragon ✅
```
[ ] ItemList.csv tem IDs 5752-5771
[ ] ItemList.bin foi recriado
[ ] Consegue spawnar items com /spawn
[ ] Items aparecem no inventário
[ ] Stats dos items estão corretos
```

---

## 🎯 Comandos Úteis de GM

### Teleporte
```
/move X Y         # Teleportar para coordenada
/movechar NOME X Y # Teleportar outro player
```

### Items
```
/spawn ID QTD     # Spawnar item
/itemall          # Receber todos items do jogo
```

### Level & Stats
```
/level NIVEL      # Setar level
/str VALOR        # Adicionar STR
/int VALOR        # Adicionar INT
/dex VALOR        # Adicionar DEX
/con VALOR        # Adicionar CON
```

### Combate
```
/kill             # Matar target
/killall          # Matar todos mobs na tela
/god              # Modo imortal
```

### Outros
```
/notice TEXTO     # Enviar mensagem global
/kick NOME        # Kickar player
/ban NOME         # Banir player
/reload items     # Recarregar ItemList.csv
```

---

## 📝 Logs Importantes

### DBSrv Logs
```
Server/DBSrv/run/output.txt       # Log geral
Server/DBSrv/run/Log/             # Logs por dia
```

### TMSrv Logs
```
Server/TMSrv/run/                 # Logs na pasta run
(procurar por .txt ou .log)
```

### Cliente Logs
```
Cliente/                          # Pode ter logs de erro
(procurar por error.log, crash.dmp)
```

---

## 🎓 Próximos Passos Após Setup

### 1. Integrar Sistema RedDragon ao VS
- Seguir: `INTEGRATION_GUIDE.md`
- Compilar código
- Testar Dragon Infusion in-game

### 2. Criar Assets Visuais
- Contratar 3D artist
- Seguir: `RedDragon_Client_Implementation_Guide.md`

### 3. Implementar Boss AI
- Criar arquivo `RedDragonIgnis.cpp`
- Configurar spawn automático

### 4. Beta Testing
- Convidar amigos
- Coletar feedback
- Ajustar balanceamento

---

## 🆘 Suporte

### Recursos Úteis
- **MySQL Workbench**: https://dev.mysql.com/downloads/workbench/
- **Port Checker**: https://www.yougetsignal.com/tools/open-ports/
- **IP Público**: https://meuip.com.br/

### Documentação do Projeto
- **PROJECT_STATUS.md**: Estado atual do projeto
- **INTEGRATION_GUIDE.md**: Como integrar ao Visual Studio
- **REDDRAGON_README.md**: Guia geral do sistema RedDragon

---

**Versão**: 1.0
**Data**: 2025-01-31
**IP do Sistema**: 21.0.0.140
**Status**: ✅ Guia Completo

🎮 **Divirta-se testando o WYD!** 🎮
