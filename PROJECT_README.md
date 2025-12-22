# 💰 FinançaCasal - Gestão Financeira para Casais

<div align="center">

Uma aplicação web moderna para controle financeiro compartilhado entre casais, construída com React, TypeScript e Node.js.

[Demo](#) | [Documentação](./docs/SETUP.md) | [Contribuir](#)

</div>

## ✨ Funcionalidades

### ✅ Implementadas (Fase 1)

- **Autenticação Completa**
  - Registro e login de usuários
  - JWT authentication
  - Proteção de rotas
  - Gerenciamento de sessão

- **Layout Responsivo**
  - Sidebar navegável
  - Header com menu de usuário
  - Mobile-first design
  - Dark mode (preparado)

- **Dashboard Base**
  - Cards de resumo financeiro
  - Estrutura para gráficos
  - Interface limpa e moderna

### 🚧 Em Desenvolvimento (Fase 2)

- **Transações**
  - CRUD completo
  - Categorização
  - Sistema de divisão 50/50
  - Upload de anexos
  - Filtros e busca

- **Categorias**
  - Categorias pré-definidas com ícones
  - Criação de categorias personalizadas
  - Subcategorias
  - Orçamento por categoria

### 📋 Planejadas (Fase 3)

- **Assinaturas Recorrentes**
  - Dashboard de assinaturas
  - Alertas de vencimento
  - Gestão de gastos fixos

- **Metas Financeiras**
  - Definição de objetivos
  - Acompanhamento de progresso
  - Contribuições mensais

- **Relatórios e Análises**
  - Gráficos interativos
  - Comparativos mensais
  - Insights inteligentes
  - Exportação CSV/PDF

- **Sistema de Parceiros**
  - Convite por email/código
  - Divisão customizada de despesas
  - Balanço entre parceiros

## 🛠️ Stack Tecnológico

### Frontend
- **React 18** - UI Library
- **TypeScript** - Type safety
- **Vite** - Build tool
- **Tailwind CSS** - Styling
- **shadcn/ui** - Component library
- **Zustand** - State management
- **React Router** - Routing
- **Lucide React** - Icons
- **Recharts** - Charts (preparado)
- **React Hook Form + Zod** - Forms & validation (preparado)

### Backend
- **Node.js** - Runtime
- **Express** - Web framework
- **TypeScript** - Type safety
- **Prisma** - ORM
- **PostgreSQL** - Database
- **JWT** - Authentication
- **bcrypt** - Password hashing

## 📁 Estrutura do Projeto

```
projeto-wyd/
├── frontend/              # Aplicação React
│   ├── src/
│   │   ├── components/   # Componentes reutilizáveis
│   │   │   ├── ui/       # shadcn/ui components
│   │   │   ├── layout/   # Layout components
│   │   │   └── auth/     # Auth components
│   │   ├── pages/        # Páginas da aplicação
│   │   ├── stores/       # Zustand stores
│   │   ├── lib/          # Utilidades
│   │   └── types/        # TypeScript types
│   └── package.json
│
├── backend/              # API Node.js
│   ├── src/
│   │   ├── controllers/  # Route controllers
│   │   ├── routes/       # API routes
│   │   ├── middleware/   # Middlewares
│   │   ├── lib/          # Libraries
│   │   └── utils/        # Utilities
│   ├── prisma/          # Database schema
│   └── package.json
│
└── docs/                # Documentação
    └── SETUP.md         # Guia de instalação
```

## 🚀 Quick Start

### Pré-requisitos

- Node.js 18+
- PostgreSQL 14+
- npm ou yarn

### Instalação Rápida

```bash
# Backend
cd backend
npm install
cp .env.example .env
# Configure .env com suas credenciais
npm run prisma:migrate
npm run dev

# Frontend (em outro terminal)
cd frontend
npm install
npm run dev
```

📖 **Guia Completo:** Consulte [docs/SETUP.md](./docs/SETUP.md) para instruções detalhadas.

## 🎨 Design

### Paleta de Cores

- **Primary:** #6366F1 (Índigo)
- **Success:** #10B981 (Verde)
- **Danger:** #EF4444 (Vermelho)
- **Warning:** #F59E0B (Âmbar)
- **Background:** #F9FAFB

### Princípios de Design

- Interface limpa e minimalista
- Mobile-first e totalmente responsivo
- Feedback visual imediato
- Animações sutis
- Acessibilidade (WCAG 2.1)

## 📊 Status do Desenvolvimento

| Funcionalidade | Status |
|---------------|--------|
| Autenticação | ✅ Completo |
| Layout Base | ✅ Completo |
| Dashboard | 🟡 Básico |
| Transações | 🔴 Pendente |
| Categorias | 🔴 Pendente |
| Assinaturas | 🔴 Pendente |
| Metas | 🔴 Pendente |
| Relatórios | 🔴 Pendente |
| Sistema de Parceiros | 🔴 Pendente |

**Legenda:** ✅ Completo | 🟡 Em andamento | 🔴 Pendente

## 🔐 Segurança

- ✅ Senhas hasheadas com bcrypt
- ✅ JWT com expiração
- ✅ Proteção contra XSS
- ✅ CORS configurado
- ✅ Validação server-side
- ✅ SQL injection protection (Prisma)
- 🚧 Rate limiting (planejado)
- 🚧 CSRF tokens (planejado)

## 🤝 Contribuindo

Contribuições são bem-vindas! Sinta-se à vontade para:

1. Fork o projeto
2. Criar uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abrir um Pull Request

## 📝 License

Este projeto está sob a licença MIT. Veja o arquivo [LICENSE](LICENSE) para mais detalhes.

## 👥 Autores

- Desenvolvido como projeto educacional de gestão financeira

## 🙏 Agradecimentos

- [shadcn/ui](https://ui.shadcn.com/) - Componentes UI
- [Lucide Icons](https://lucide.dev/) - Ícones
- [Tailwind CSS](https://tailwindcss.com/) - Framework CSS
- [Prisma](https://www.prisma.io/) - ORM

---

<div align="center">

Feito com ❤️ para casais que querem organizar suas finanças juntos

</div>
