import { Router } from 'express'
import { authenticate } from '../middleware/auth.middleware'

const router = Router()

router.use(authenticate)

router.get('/', (req, res) => res.json({ message: 'Get categories' }))
router.post('/', (req, res) => res.json({ message: 'Create category' }))
router.put('/:id', (req, res) => res.json({ message: 'Update category' }))
router.delete('/:id', (req, res) => res.json({ message: 'Delete category' }))

export default router
