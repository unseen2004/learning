import express, { Request, Response, NextFunction, RequestHandler } from 'express'
import router from './router'
import morgan from 'morgan'
import cors from 'cors'
import path from 'path'
import { protect } from './modules/auth'
import { createNewUser, signin } from './handlers/user'

const app = express()

app.use(cors())
app.use(morgan('dev'))
app.use(express.json())
app.use(express.urlencoded({ extended: true }))
app.use('/uploads', express.static(path.join(__dirname, '../uploads')))

const requestHandler = (fn: (req: Request, res: Response) => Promise<any>): RequestHandler => {
  return (req, res, next) => fn(req, res);
};

app.get('/', (req: Request, res: Response, next: NextFunction) => {
   res.send('Hello World');
})

app.use('/api', protect, router)

app.post('/user', requestHandler(createNewUser))
app.post('/signin', requestHandler(signin))

app.use((err: Error, req: Request, res: Response, next: NextFunction) => {
    console.log(err)
    res.json({ message: `had an error: ${err.message}` })
})

export default app