import prisma from '../db'
import { Request, Response } from 'express'
import { comparePasswords, createJWT, hashPassword } from '../modules/auth'

export const createNewUser = async (req: Request, res: Response) => {
  try {
    const user = await prisma.user.create({
      data: {
        username: req.body.username,
        password: await hashPassword(req.body.password)
      }
    })

    const token = createJWT(user)
    res.json({ token })
  } catch (e: any) {
    if (e.code === 'P2002' && e.meta?.target?.includes('username')) {
      res.status(400).json({ message: 'Username already exists' })
    } else {
      console.error('Error creating user:', e)
      res.status(500).json({ message: 'Internal server error' })
    }
  }
}

export const signin = async (req: Request, res: Response) => {
    const user = await prisma.user.findUnique({
        where: {
            username: req.body.username
        }
    })

    if (!user) {
        res.status(401)
        return res.json({ message: 'Invalid credentials' })
    }

    const isValid = await comparePasswords(req.body.password, user.password)

    if (!isValid) {
        res.status(401)
        return res.json({ message: 'Invalid credentials' })
    }

    const token = createJWT(user)
    res.json({ token })
}