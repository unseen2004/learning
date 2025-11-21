import { Router, Response, Request, RequestHandler } from 'express'
import { body } from "express-validator"
import { handleInputErrors } from './modules/middleware'
import {
  getCocktails, getOneCocktail, createCocktail,
  updateCocktail, deleteCocktail,
  addIngredientToCocktail, updateCocktailIngredient, removeCocktailIngredient
} from './handlers/cocktail'
import {
  getIngredients, getOneIngredient, createIngredient,
  updateIngredient, deleteIngredient
} from './handlers/ingredient'
import { AuthRequest } from './modules/types'
import { upload } from './modules/upload'

const router = Router()

const authHandler = (fn: (req: AuthRequest, res: Response) => Promise<any>): RequestHandler => {
  return (req, res, next) => fn(req as AuthRequest, res);
};

const requestHandler = (fn: (req: Request, res: Response) => Promise<any>): RequestHandler => {
  return (req, res, next) => fn(req, res);
};

/**
 * Cocktail
 */
router.get('/cocktail', authHandler(getCocktails))
router.get('/cocktail/:id', requestHandler(getOneCocktail))
router.post('/cocktail',
    body('name').isString(),
    body('category').isString(),
    body('instruction').isString(),
    handleInputErrors,
    authHandler(createCocktail)
)
router.put('/cocktail/:id',
    body('name').optional().isString(),
    body('category').optional().isString(),
    body('instruction').optional().isString(),
    handleInputErrors,
    authHandler(updateCocktail)
)
router.delete('/cocktail/:id', authHandler(deleteCocktail))

/**
 * Ingredient
 */
router.get('/ingredient', authHandler(getIngredients))
router.get('/ingredient/:id', requestHandler(getOneIngredient))
router.post('/ingredient',
    upload.single('photo'),
    body('name').isString(),
    body('description').isString(),
    body('alcohol').isBoolean(),
    handleInputErrors,
    authHandler(createIngredient)
)
router.put('/ingredient/:id',
    upload.single('photo'),
    body('name').optional().isString(),
    body('description').optional().isString(),
    body('alcohol').optional().isBoolean(),
    handleInputErrors,
    authHandler(updateIngredient)
)
router.delete('/ingredient/:id', authHandler(deleteIngredient))

/**
 * CocktailIngredient
 */
router.post('/cocktail/:cocktailId/ingredient',
    body('ingredientId').isInt(),
    body('quantity').isString(),
    handleInputErrors,
    authHandler(addIngredientToCocktail)
)
router.put('/cocktail/:cocktailId/ingredient/:ingredientId',
    body('quantity').isString(),
    handleInputErrors,
    authHandler(updateCocktailIngredient)
)
router.delete('/cocktail/:cocktailId/ingredient/:ingredientId', authHandler(removeCocktailIngredient))

export default router