import request from 'supertest';
import app from '../../src/server';
import { testUser } from '../fixtures/users';
import { mojito, margarita } from '../fixtures/cocktails';

describe('Cocktail API', () => {
  let token: string;

  beforeEach(async () => {
    const res = await request(app)
      .post('/user')
      .send(testUser);

    token = res.body.token;
  });

  it('should create a new cocktail', async () => {
    const res = await request(app)
      .post('/api/cocktail')
      .set('Authorization', `Bearer ${token}`)
      .send(mojito)
      .expect(200);

    expect(res.body.data).toHaveProperty('id');
    expect(res.body.data.name).toBe(mojito.name);
  });

  it('should get all cocktails for authenticated user', async () => {
    await request(app)
      .post('/api/cocktail')
      .set('Authorization', `Bearer ${token}`)
      .send(mojito);

    const res = await request(app)
      .get('/api/cocktail')
      .set('Authorization', `Bearer ${token}`)
      .expect(200);

    expect(Array.isArray(res.body.data)).toBe(true);
    expect(res.body.data.length).toBeGreaterThan(0);
  });

  it('should get a specific cocktail by ID', async () => {
    const createRes = await request(app)
      .post('/api/cocktail')
      .set('Authorization', `Bearer ${token}`)
      .send(mojito);

    const cocktailId = createRes.body.data.id;

    const res = await request(app)
      .get(`/api/cocktail/${cocktailId}`)
      .set('Authorization', `Bearer ${token}`)
      .expect(200);

    expect(res.body.data.id).toBe(cocktailId);
    expect(res.body.data.name).toBe(mojito.name);
  });

  it('should update a cocktail', async () => {
    const createRes = await request(app)
      .post('/api/cocktail')
      .set('Authorization', `Bearer ${token}`)
      .send(mojito);

    const cocktailId = createRes.body.data.id;

    const updatedData = { instruction: 'Updated instructions' };
    const res = await request(app)
      .put(`/api/cocktail/${cocktailId}`)
      .set('Authorization', `Bearer ${token}`)
      .send(updatedData)
      .expect(200);

    expect(res.body.data.instruction).toBe(updatedData.instruction);
  });

  it('should delete a cocktail', async () => {
    const createRes = await request(app)
      .post('/api/cocktail')
      .set('Authorization', `Bearer ${token}`)
      .send(mojito);

    const cocktailId = createRes.body.data.id;

    await request(app)
      .delete(`/api/cocktail/${cocktailId}`)
      .set('Authorization', `Bearer ${token}`)
      .expect(200);

    await request(app)
      .get(`/api/cocktail/${cocktailId}`)
      .set('Authorization', `Bearer ${token}`)
      .expect(404);
  });
});