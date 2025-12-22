# Guia de Instalação - FinançaCasal

## 📋 Pré-requisitos

- Node.js 18+ instalado
- PostgreSQL 14+ instalado e rodando
- npm ou yarn
- Git

## 🚀 Instalação

### 1. Clone o repositório

```bash
git clone <repository-url>
cd projeto-wyd
```

### 2. Configuração do Backend

```bash
cd backend

# Instalar dependências
npm install

# Configurar variáveis de ambiente
cp .env.example .env
```

Edite o arquivo `.env` e configure as variáveis:

```env
DATABASE_URL="postgresql://user:password@localhost:5432/finance_casal?schema=public"
JWT_SECRET="your-super-secret-jwt-key-change-this"
JWT_EXPIRES_IN="7d"
PORT=3001
NODE_ENV="development"
CORS_ORIGIN="http://localhost:5173"
```

```bash
# Criar banco de dados PostgreSQL
createdb finance_casal

# Executar migrations do Prisma
npm run prisma:migrate

# Gerar Prisma Client
npm run prisma:generate

# Iniciar servidor de desenvolvimento
npm run dev
```

O backend estará rodando em `http://localhost:3001`

### 3. Configuração do Frontend

Em outro terminal:

```bash
cd frontend

# Instalar dependências
npm install

# Criar arquivo .env (opcional)
echo "VITE_API_URL=http://localhost:3001/api" > .env

# Iniciar servidor de desenvolvimento
npm run dev
```

O frontend estará rodando em `http://localhost:5173`

## 🗄️ Configuração do Banco de Dados

### Criar banco manualmente:

```sql
CREATE DATABASE finance_casal;
```

### Executar migrations:

```bash
cd backend
npm run prisma:migrate
```

### Visualizar dados (Prisma Studio):

```bash
npm run prisma:studio
```

Abrirá em `http://localhost:5555`

## 📱 Acessando a Aplicação

1. Acesse `http://localhost:5173`
2. Clique em "Cadastre-se" para criar uma conta
3. Preencha seus dados e crie sua conta
4. Você será redirecionado para o dashboard

## 🔧 Scripts Disponíveis

### Backend

```bash
npm run dev          # Inicia servidor em modo desenvolvimento
npm run build        # Compila TypeScript
npm run start        # Inicia servidor em produção
npm run prisma:migrate   # Executa migrations
npm run prisma:generate  # Gera Prisma Client
npm run prisma:studio    # Abre Prisma Studio
```

### Frontend

```bash
npm run dev      # Inicia servidor de desenvolvimento
npm run build    # Faz build para produção
npm run preview  # Preview do build de produção
```

## 🐛 Troubleshooting

### Erro de conexão com banco de dados

- Verifique se o PostgreSQL está rodando
- Confirme as credenciais no DATABASE_URL
- Teste a conexão: `psql -U user -d finance_casal`

### Erro "Failed to fetch sha256 checksum" (Prisma)

```bash
export PRISMA_ENGINES_CHECKSUM_IGNORE_MISSING=1
npm run prisma:generate
```

### Erro de CORS

- Verifique se CORS_ORIGIN no backend está configurado corretamente
- Deve ser `http://localhost:5173` em desenvolvimento

### Porta já em uso

Se a porta 3001 ou 5173 já estiver em uso:

Backend (`.env`):
```env
PORT=3002
```

Frontend (`vite.config.ts`):
```ts
server: {
  port: 5174
}
```

## 📚 Próximos Passos

Após a instalação, você pode:

1. Explorar o Dashboard
2. Adicionar transações (em breve)
3. Convidar seu parceiro (em breve)
4. Configurar categorias personalizadas (em breve)

## 🔐 Usuário de Teste

Durante o desenvolvimento, você pode criar usuários de teste manualmente através do endpoint `/api/auth/register`.

## 📞 Suporte

Para problemas ou dúvidas, abra uma issue no repositório.
