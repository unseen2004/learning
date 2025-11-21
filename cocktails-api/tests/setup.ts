import prisma from '../src/db';
import dotenv from 'dotenv';

dotenv.config({ path: '.env.test' });

beforeEach(async () => {
  await prisma.cocktailIngredient.deleteMany({});
  await prisma.ingredient.deleteMany({});
  await prisma.cocktail.deleteMany({});
  await prisma.user.deleteMany({});
});

afterAll(async () => {
  await prisma.$disconnect();
});