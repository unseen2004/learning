import prisma from "../db"
import { Request, Response } from "express"
import { AuthRequest } from "../modules/types"
import { upload } from '../modules/upload'

export const getOneIngredient = async (req: Request, res: Response) => {
  const ingredient = await prisma.ingredient.findUnique({
    where: {
      id: parseInt(req.params.id)
    }
  })

  if (!ingredient) {
    return res.status(404).json({ message: 'Ingredient not found' })
  }

  res.json({ data: ingredient })
}

export const getIngredients = async (req: AuthRequest, res: Response) => {
  const ingredients = await prisma.ingredient.findMany({
    where: {
      userId: req.user.id
    }
  })

  res.json({ data: ingredients })
}

export const createIngredient = async (req: AuthRequest, res: Response) => {
  let photoPath = null
  if (req.file) {
    photoPath = `/uploads/${req.file.filename}`;
  }

  const ingredient = await prisma.ingredient.create({
    data: {
      name: req.body.name,
      description: req.body.description,
      alcohol: req.body.alcohol === 'true' || req.body.alcohol === true,
      photo: photoPath,
      userId: req.user.id
    }
  })

  res.json({ data: ingredient })
}

export const updateIngredient = async (req: AuthRequest, res: Response) => {
  const ingredient = await prisma.ingredient.findUnique({
    where: {
      id: parseInt(req.params.id)
    }
  })

  if (!ingredient || ingredient.userId !== req.user.id) {
    return res.status(404).json({ message: 'Not found or not authorized' })
  }

  const data = { ...req.body };

  if (data.alcohol !== undefined) {
    data.alcohol = data.alcohol === 'true' || data.alcohol === true;
  }

  if (req.file) {
    data.photo = `/uploads/${req.file.filename}`;
  }

  const updatedIngredient = await prisma.ingredient.update({
    where: {
      id: parseInt(req.params.id)
    },
    data
  })

  res.json({ data: updatedIngredient })
}

export const deleteIngredient = async (req: AuthRequest, res: Response) => {
  const ingredient = await prisma.ingredient.findUnique({
    where: {
      id: parseInt(req.params.id)
    }
  })

  if (!ingredient || ingredient.userId !== req.user.id) {
    return res.status(404).json({ message: 'Not found or not authorized' })
  }

  const deleted = await prisma.ingredient.delete({
    where: {
      id: parseInt(req.params.id)
    }
  })

  res.json({ data: deleted })
}