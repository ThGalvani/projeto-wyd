import { Router } from 'express'
import { authenticate } from '../middleware/auth.middleware'

const router = Router()

router.use(authenticate)

router.post('/invite', (req, res) => res.json({ message: 'Invite partner' }))
router.post('/accept', (req, res) => res.json({ message: 'Accept partner invite' }))
router.delete('/disconnect', (req, res) => res.json({ message: 'Disconnect partner' }))
router.get('/balance', (req, res) => res.json({ message: 'Get partner balance' }))

export default router
