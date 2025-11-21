import request from 'supertest';
import app from '../../src/server';
import { testUser } from '../fixtures/users';

describe('Authentication', () => {
  it('should register a new user', async () => {
    const res = await request(app)
      .post('/user')
      .send(testUser)
      .expect(200);

    expect(res.body).toHaveProperty('token');
  });

  it('should allow login with valid credentials', async () => {
    await request(app)
      .post('/user')
      .send(testUser);

    const res = await request(app)
      .post('/signin')
      .send(testUser)
      .expect(200);

    expect(res.body).toHaveProperty('token');
  });

  it('should reject login with invalid credentials', async () => {
    await request(app)
      .post('/user')
      .send(testUser);

    await request(app)
      .post('/signin')
      .send({
        username: testUser.username,
        password: 'wrongpassword'
      })
      .expect(401);
  });
});