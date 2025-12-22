# Guia de Deploy no Vercel - FinançaCasal

## 🚀 Deploy Automático

### 1. Frontend (Vite + React)

#### Via Vercel Dashboard:

1. Acesse [vercel.com](https://vercel.com)
2. Click "Add New Project"
3. Importe o repositório `couplesfinance`
4. Configure:
   - **Framework Preset:** Vite
   - **Root Directory:** `frontend`
   - **Build Command:** `npm run build`
   - **Output Directory:** `dist`
   - **Install Command:** `npm install`

#### Variáveis de Ambiente (Frontend):
```env
VITE_API_URL=https://seu-backend.vercel.app/api
```

### 2. Backend (Node.js + Express)

#### Via Vercel Dashboard:

1. Crie um NOVO projeto para o backend
2. Importe o mesmo repositório `couplesfinance`
3. Configure:
   - **Root Directory:** `backend`
   - **Build Command:** `npm run build`
   - **Output Directory:** `dist`

#### Variáveis de Ambiente (Backend):
```env
# Deixe vazio para rodar em modo DEMO (sem banco de dados)
DATABASE_URL=

# JWT Secret (IMPORTANTE: Gere um secret seguro)
JWT_SECRET=sua-chave-super-secreta-aqui-12345

# CORS (URL do frontend)
CORS_ORIGIN=https://seu-frontend.vercel.app

# Demo Mode
DEMO_MODE=true
```

## 🎭 Modo Demo

O backend está configurado para rodar em **modo demo** sem necessidade de banco de dados!

### Como funciona:

- ✅ Se `DATABASE_URL` não estiver configurado → **Modo Demo**
- ✅ Usuários são salvos em memória (Map)
- ✅ Dados resetam a cada deploy
- ✅ Perfeito para testar a aplicação

### Usuário Demo Pré-configurado:

```
Email: demo@example.com
Senha: demo123
```

### Quando usar Modo Demo:

- ✅ Preview deployments
- ✅ Testes rápidos
- ✅ Demonstrações
- ❌ Produção real (configure o banco)

## 📊 Conectar Banco de Dados Real

### Opção 1: Supabase (Recomendado)

1. Crie conta em [supabase.com](https://supabase.com)
2. Crie novo projeto
3. Vá em Settings → Database
4. Copie a Connection String (URI)
5. No Vercel, adicione a variável:

```env
DATABASE_URL=postgresql://postgres:[PASSWORD]@db.[PROJECT].supabase.co:5432/postgres
DEMO_MODE=false
```

6. Execute migrations:
```bash
cd backend
npm run prisma:migrate
```

### Opção 2: Railway

1. Crie conta em [railway.app](https://railway.app)
2. New Project → Provision PostgreSQL
3. Copie DATABASE_URL
4. Configure no Vercel

### Opção 3: Neon

1. Crie conta em [neon.tech](https://neon.tech)
2. New Project → Copy connection string
3. Configure no Vercel

## 🔄 Preview Deployments

O Vercel cria automaticamente um preview para cada branch e PR!

### Como funciona:

1. Push para qualquer branch (ex: `feature/nova-funcionalidade`)
2. Vercel cria deploy automático
3. Você recebe URL de preview: `https://projeto-abc123.vercel.app`
4. Teste suas mudanças antes do merge

### Configurar Preview do Backend:

No `frontend/vercel.json`, atualize o rewrite para apontar para o backend em preview:

```json
{
  "rewrites": [
    {
      "source": "/api/:path*",
      "destination": "https://backend-preview.vercel.app/api/:path*"
    }
  ]
}
```

## 🔗 Conectar Frontend e Backend

### Método 1: Rewrite (Recomendado)

No `frontend/vercel.json`:
```json
{
  "rewrites": [
    {
      "source": "/api/:path*",
      "destination": "https://backend-production.vercel.app/api/:path*"
    }
  ]
}
```

**Vantagem:** Não precisa configurar CORS, tudo fica no mesmo domínio.

### Método 2: Variável de Ambiente

1. Configure `VITE_API_URL` no Vercel (frontend)
2. Configure `CORS_ORIGIN` no Vercel (backend)

## 📝 Checklist de Deploy

### Frontend:
- [ ] Build passa sem erros
- [ ] `VITE_API_URL` configurado
- [ ] Rewrite para backend configurado (opcional)

### Backend:
- [ ] Build passa sem erros
- [ ] `JWT_SECRET` configurado (seguro)
- [ ] `CORS_ORIGIN` configurado
- [ ] `DEMO_MODE=true` OU `DATABASE_URL` configurado
- [ ] Migrations rodadas (se usar banco real)

## 🐛 Troubleshooting

### Erro: "Failed to fetch"

**Causa:** Frontend não consegue acessar backend

**Solução:**
1. Verifique se `VITE_API_URL` está correto
2. Verifique CORS no backend
3. Teste endpoint do backend diretamente: `https://backend.vercel.app/health`

### Erro: "Database connection failed"

**Causa:** `DATABASE_URL` inválido ou banco inacessível

**Solução:**
1. Verifique connection string
2. Use modo demo temporariamente: `DEMO_MODE=true`
3. Teste conexão local primeiro

### Erro: "401 Unauthorized"

**Causa:** Problema com JWT

**Solução:**
1. Verifique se `JWT_SECRET` está configurado
2. Limpe localStorage no navegador
3. Faça logout/login novamente

## 📈 Monitoramento

### Logs do Vercel:

1. Acesse seu projeto no Vercel
2. Vá em "Deployments"
3. Click em qualquer deployment
4. Veja "Runtime Logs"

### Health Check:

- Frontend: `https://seu-app.vercel.app`
- Backend: `https://backend.vercel.app/health`
- Status API: `https://backend.vercel.app/api/status`

## 🎯 URLs de Exemplo

Após deploy bem-sucedido:

```
Frontend Produção: https://couplesfinance.vercel.app
Backend Produção:  https://couplesfinance-api.vercel.app

Preview (branch):  https://couplesfinance-git-feature-abc.vercel.app
Health Check:      https://couplesfinance-api.vercel.app/health
```

## 💡 Dicas

1. **Use modo demo** para testar rapidamente sem configurar banco
2. **Gere JWT_SECRET forte:** `openssl rand -base64 32`
3. **Monitore logs** após cada deploy
4. **Teste preview deployments** antes de fazer merge
5. **Configure domínio customizado** nas configurações do Vercel

## 📚 Links Úteis

- [Vercel Docs](https://vercel.com/docs)
- [Supabase Docs](https://supabase.com/docs)
- [Prisma Deployment](https://www.prisma.io/docs/guides/deployment)
