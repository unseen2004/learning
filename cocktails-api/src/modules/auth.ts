import jwt from 'jsonwebtoken'
import bcrypt from 'bcrypt'
import { Request, Response, NextFunction } from 'express'
import { User, AuthRequest } from './types'

export const comparePasswords = (password: string, hash: string) => {
  return bcrypt.compare(password, hash)
}

export const hashPassword = (password: string) => {
  return bcrypt.hash(password, 5)
}

export const createJWT = (user: User) => {
  const secret = process.env.JWT_SECRET
  if (!secret) {
    throw new Error('JWT_SECRET is not defined in environment variables')
  }

  const token = jwt.sign(
    {
      id: user.id,
      username: user.username
    },
    secret
  )
  return token
}

export const protect = (req: Request, res: Response, next: NextFunction) => {
  const bearer = req.headers.authorization

  if (!bearer) {
    res.status(401)
    res.json({ message: 'not authorized' })
    return
  }

  const [, token] = bearer.split(' ')

  if (!token) {
    res.status(401)
    res.json({ message: 'not valid token' })
    return
  }

  try {
    const secret = process.env.JWT_SECRET
    if (!secret) {
      throw new Error('JWT_SECRET is not defined')
    }

    const payload = jwt.verify(token, secret)
    const user = (payload as unknown) as User

    (req as AuthRequest).user = user
    next()
  } catch (e) {
    console.error(e)
    res.status(401)
    res.json({ message: 'not valid token' })
    return
  }
}